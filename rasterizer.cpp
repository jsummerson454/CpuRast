#include "tgaimage.h"
#include <glm/glm.hpp>
#include "rasterizer.h"

// Bresenham's line drawing algorithm (see https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm)
// Since Bresenham's algorithm in its base form is only defined over a single octant of lines (those 
// with slope between 0 and 1), we must transform the other cases into this octant.
void drawLine(int x0, int y0, int x1, int y1, TGAImage& img, const TGAColor& col) {
    // check for "steep" lines (slope > 1), if so simply draw transposally
    bool transpose = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        transpose = true;
    }
    // since line must be drawn in increasing order
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    // cover slopes between 0 and -1 by tracking if y is increasing or decreasing
    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int D = (2 * dy) - dx;
    int y = y0;
    for (int x = x0; x < x1; ++x) {
        if (transpose) {
            img.set(y, x, col);
        }
        else {
            img.set(x, y, col);
        }
        if (D > 0) {
            y += yi;
            D += (2 * (dy - dx));
        }
        else {
            D += (2 * dy);
        }
    }
}

// return edge orientation function (+ve if "inside" edge), also relates to barycentric coordinates
// since this is proportional to the area of the triangle ABP (specifically 2x area of triangle)
int edge2d(ipoint2d const& a, ipoint2d const& b, ipoint2d const& p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

// Note that this triangle drawing method is replicating the behaviour of how a GPU (massively parallel)
// would draw a triangle, and hence is not well suited for serial CPU rendering - a scanline rasterization
// approach would be more efficient in serial software rendering explicitly, but its poor parallelisation
// means that for any SIMD capable processor or hardware implementation this approach is preferred.
void drawTriangle(ipoint2d const& a, ipoint2d const& b, ipoint2d const& c,
                  zbuffer_t depthA, zbuffer_t depthB, zbuffer_t depthC,
                  zbuffer_t* zbuffer, TGAImage& img, const TGAColor& col) {

    int area = edge2d(a, b, c);
    // if area 0 then degenerate, if area <0 then backfacing (assuming all triangles correctly
    // wound CCW), so reject early
    if (area <= 0) {
        return;
    }
    float normFactor = 1.0f/area;

    // compute bounding box of triangle
    ipoint2d bbMin = ipoint2d{ std::min(std::min(a.x, b.x), c.x),
        std::min(std::min(a.y, b.y), c.y) };
    ipoint2d bbMax = ipoint2d{ std::max(std::max(a.x, b.x), c.x),
        std::max(std::max(a.y, b.y), c.y) };

    // clip to image dimensions
    bbMin.x = std::max(bbMin.x, 0);
    bbMin.y = std::max(bbMin.y, 0);
    bbMax.x = std::min(bbMax.x, img.get_width() - 1);
    bbMax.y = std::min(bbMax.y, img.get_height() - 1);


    // Iterate over every pixel in bounding box, if pixel is within triangle (determined via edge signed 
    // distance functions, which closely relate to barycentric coordinates) then draw it.
    ipoint2d p{};
    for (p.y = bbMin.y; p.y <= bbMax.y; p.y++) {
        for (p.x = bbMin.x; p.x <= bbMax.x; p.x++) {
            // note - could just do edge funcs for 2 of these and normalize using 2x triangle area, or 
            // compute all 3 and normalize sum to 1 to convert into barycentric coordinates - same
            // computationally as the triangle area sum is effectively just another call to edge2d...
            int wa = edge2d(b, c, p);
            int wb = edge2d(c, a, p);
            int wc = edge2d(a, b, p);

            if (wa >= 0 && wb >= 0 && wc >= 0) {
                // check against zbuffer, only write if less than zbuffer, then update it
                // note we can simply interpolate Z as normal here since we are not working with
                // world space Z values, but rather the (mapped) NDC Z values
                float ba = wa * normFactor;
                float bb = wb * normFactor;
                float bc = wc * normFactor;
                // 0.5 added to allow proper rounding when casting from float to zbuffer_t (unsigned integer)
                zbuffer_t z = (zbuffer_t) (ba * depthA + bb * depthB + bc * depthC + 0.5);
                if (z <= zbuffer[p.y * img.get_width() + p.x]) {
                    img.set(p.x, p.y, col);
                    zbuffer[p.y * img.get_width() + p.x] = z;
                }
            }
        }
    }
}