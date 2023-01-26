#pragma once
#include "External/tgaimage.h"
#include "shaderProgram.h"
#include <vector>

typedef uint16_t zbuffer_t;
constexpr auto ZBUFFMAX = std::numeric_limits<zbuffer_t>::max();

struct ipoint2d {
	int x, y;
};

template <typename Vertex, typename Varying>
class Renderer {
	TGAImage m_image;
	int m_width, m_height;
	zbuffer_t* m_zbuffer;
	void draw_triangle(IShaderProgram<Vertex, Varying>& shaderProgram, Varying& a, Varying& b, Varying& c);
	std::vector<Varying> processVertices(IShaderProgram<Vertex, Varying>& shaderProgram, std::vector<Vertex>& vertexBuffer);
	int edge2d(ipoint2d const& a, ipoint2d const& b, ipoint2d const& p);
public:
	Renderer(int width, int height);
	~Renderer();
	void draw(IShaderProgram<Vertex, Varying>& shaderProgram, std::vector<Vertex>& vertexBuffer, std::vector<int>& indexBuffer);
};

// edge orientation function (+ve if "inside" edge), also relates to barycentric coordinates
// since this is proportional to the area of the triangle ABP (specifically 2x area of triangle)
template<typename Vertex, typename Varying>
inline int Renderer<Vertex, Varying>::edge2d(ipoint2d const& a, ipoint2d const& b, ipoint2d const& p) {
	return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

template<typename Vertex, typename Varying>
inline Renderer<Vertex, Varying>::Renderer(int width, int height)
{
	m_zbuffer = new zbuffer_t[width * height];
	for (int i = 0; i < width * height; i++) {
		m_zbuffer[i] = ZBUFFMAX;
	}
	m_image = TGAImage(width, height, TGAImage::RGB);
	m_width = width;
	m_height = height;
}

template<typename Vertex, typename Varying>
inline Renderer<Vertex, Varying>::~Renderer()
{
	delete[] m_zbuffer;
}

// TODO: consider adding a Buffer class rather than passing a vertex and index buffer, then can maybe just use a
// get next triangle function or something instead of having to overload the function for an unindexed verison...
template<typename Vertex, typename Varying>
inline void Renderer<Vertex, Varying>::draw(IShaderProgram<Vertex, Varying>& shaderProgram, std::vector<Vertex>& vertexBuffer, std::vector<int>& indexBuffer)
{
	// Vertex processing stage (vertex shader, perspective divide, viewport transformation)
	std::vector<Varying> processedVertices = processVertices(shaderProgram, vertexBuffer);

	// Read each triangle from the index buffer and rasterize it
	for (int i = 0; i < indexBuffer.size() - 2; i += 3) {
		draw_triangle(shaderProgram, processedVertices[i], processedVertices[i + 1], processedVertices[i + 2]);
	}

	m_image.flip_vertically(); // so that origin (0,0) is bottom left, not top left
	m_image.write_tga_file("Output\\refactored.tga");
}

template<typename Vertex, typename Varying>
inline std::vector<Varying> Renderer<Vertex, Varying>::processVertices(IShaderProgram<Vertex, Varying>& shaderProgram, std::vector<Vertex>& vertexBuffer)
{
	std::vector<Varying> ret = std::vector<Varying>(vertexBuffer.size());
	for (int i = 0; i < vertexBuffer.size(); i++) {
		// vertex shader
		ret[i] = shaderProgram.vertexShader(vertexBuffer[i]);

		// perspective divide
		ret[i].gl_Position.x = ret[i].gl_Position.x / ret[i].gl_Position.w;
		ret[i].gl_Position.y = ret[i].gl_Position.y / ret[i].gl_Position.w;
		ret[i].gl_Position.z = ret[i].gl_Position.z / ret[i].gl_Position.w;
		ret[i].gl_Position.w = 1.f / ret[i].gl_Position.w;

		// TODO: CLIPPING AND CULLING STAGE GOES HERE

		// viewport transform from NDC [-1,1] to screenspace ([0, width], [0, height], [0, 1]) coordinates
		ret[i].gl_Position.x = (ret[i].gl_Position.x + 1.f) * m_width / 2.f;
		ret[i].gl_Position.y = (ret[i].gl_Position.y + 1.f) * m_height / 2.f;
		ret[i].gl_Position.z = (ret[i].gl_Position.z + 1.f) * 0.5f;
	}
	return ret;
}


template<typename Vertex, typename Varying>
inline void Renderer<Vertex, Varying>::draw_triangle(IShaderProgram<Vertex, Varying>& shaderProgram, Varying& a, Varying& b, Varying& c)
{
	// snap triangle corners to integer pixel grid
	// TODO: sub-pixel precision (e.g. 28.4 or 26.6 fixed point integer arithmetic) - do not forget to evaluate
	// at pixel CENTERS (e.g. x=5.5, y=6.5 instead of x=5, y=6) when performing pixel membership tests
	ipoint2d a_pos = { std::roundf(a.gl_Position.x), std::roundf(a.gl_Position.y) };
	ipoint2d b_pos = { std::roundf(b.gl_Position.x), std::roundf(b.gl_Position.y) };
	ipoint2d c_pos = { std::roundf(c.gl_Position.x), std::roundf(c.gl_Position.y) };

	int area = edge2d(a_pos, b_pos, c_pos);
	// if area 0 then degenerate, if area <0 then backfacing (assuming all triangles correctly
	// wound CCW), so reject early
	if (area <= 0) {
		return;
	}
	float normFactor = 1.0f / area;

	// compute bounding box of triangle
	ipoint2d bbMin = ipoint2d{ std::min(std::min(a_pos.x, b_pos.x), c_pos.x),
		std::min(std::min(a_pos.y, b_pos.y), c_pos.y) };
	ipoint2d bbMax = ipoint2d{ std::max(std::max(a_pos.x, b_pos.x), c_pos.x),
		std::max(std::max(a_pos.y, b_pos.y), c_pos.y) };

	// clip to image dimensions
	bbMin.x = std::max(bbMin.x, 0);
	bbMin.y = std::max(bbMin.y, 0);
	bbMax.x = std::min(bbMax.x, m_width - 1);
	bbMax.y = std::min(bbMax.y, m_height - 1);

	// Iterate over every pixel in bounding box, if pixel is within triangle (determined via edge signed 
	// distance functions, which closely relate to barycentric coordinates) then draw it.
	ipoint2d p{};
	for (p.y = bbMin.y; p.y <= bbMax.y; p.y++) {
		for (p.x = bbMin.x; p.x <= bbMax.x; p.x++) {
			int wa = edge2d(b_pos, c_pos, p);
			int wb = edge2d(c_pos, a_pos, p);
			int wc = edge2d(a_pos, b_pos, p);

			if (wa >= 0 && wb >= 0 && wc >= 0) {
				// check against zbuffer, only write if less than zbuffer, then update it
				// note we can simply interpolate Z as normal here since we are not working with
				// world space Z values, but rather the (mapped) NDC Z values
				float ba = wa * normFactor;
				float bb = wb * normFactor;
				float bc = wc * normFactor;
				float z = ba * a.gl_Position.z + bb * b.gl_Position.z + bc * c.gl_Position.z;
				// TODO: ONLY lazy clip on z if near/far plane clipping was skipped, as it is wasted effort otherwise
				// Late/lazy z clipping (reject if out of NDC bounds, unnecessary if near/far clipping has been done)
				if (z < 0 || z > 1) return;
				zbuffer_t z_fixed = zbuffer_t(z * ZBUFFMAX + 0.5f);
				if (z_fixed < m_zbuffer[p.y * m_width + p.x]) {
					m_zbuffer[p.y * m_width + p.x] = z_fixed;
					// TODO: make interpolation automatic, i.e. automatically interpolate all
					// fields except gl_Position rather than forcing user to provide interpolation function
					
					// perspective correct barycentrics before interpolating varyings
					float w = ba * a.gl_Position.w + bb * b.gl_Position.w + bc * c.gl_Position.w;
					ba *= ((1.f/w) * a.gl_Position.w);
					bb *= ((1.f/w) * b.gl_Position.w);
					bc *= ((1.f/w) * c.gl_Position.w);

					Varying interpolated = shaderProgram.interpolate(a, b, c, ba, bb, bc);
					glm::vec3 col = shaderProgram.fragmentShader(interpolated);
					col = col * glm::vec3(255) + glm::vec3(0.5); // convert from [0.f,1.f] colourspace to [0, 255] for TGAColor
					m_image.set(p.x, p.y, TGAColor(col.x, col.y, col.z, 1));
				}
			}
		}
	}
}