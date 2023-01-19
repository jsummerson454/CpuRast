#pragma once

#include "tgaimage.h"
#include <glm/glm.hpp>

struct ipoint2d {
	int x, y;
};

void drawLine(int x0, int y0, int x1, int y1, TGAImage& img, const TGAColor& col);
void drawTriangle(ipoint2d const& a, ipoint2d const& b, ipoint2d const& c, TGAImage& img, const TGAColor& col);
