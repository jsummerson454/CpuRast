#pragma once

#include "tgaimage.h"
#include <glm/glm.hpp>
#include <cmath>

struct ipoint2d {
	int x, y;
};

typedef uint16_t zbuffer_t;
constexpr auto ZBUFFSCALE = std::numeric_limits<zbuffer_t>::max();

void drawLine(int x0, int y0, int x1, int y1, TGAImage& img, const TGAColor& col);
void drawTriangle(ipoint2d const& a, ipoint2d const& b, ipoint2d const& c, zbuffer_t depthA, zbuffer_t depthB, zbuffer_t depthC, zbuffer_t* zbuffer, TGAImage& img, const TGAColor& col);
