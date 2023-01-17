#pragma once

#include "tgaimage.h"
#include <glm/glm.hpp>

void drawLine(int x0, int y0, int x1, int y1, TGAImage& img, const TGAColor& col);
void drawTriangle(glm::ivec2 const& a, glm::ivec2 const& b, glm::ivec2 const& c, TGAImage& img, const TGAColor& col);
