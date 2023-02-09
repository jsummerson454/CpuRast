#include "sampler.h"
#include <algorithm>
#include <math.h>
#include <cmath>
#include <glm/glm.hpp>

const glm::vec3 Sampler::operator()(float x, float y)
{
    // if sampling coords are out of bounds ([0,1]) then wrap appropriately
    if (x > 1.f || x < 0.f || y > 1.f || y < 0.f) {
        switch (m_wrapMode) {
            case CLAMPTOEDGE:
                x = std::clamp(x, 0.0f, 1.0f);
                y = std::clamp(y, 0.0f, 1.0f);
                break;
            case REPEAT:
                x = fmod(x, 1.f);
                x += (x < 0 ? 1.f : 0.f);
                y = fmod(y, 1.f);
                y += (y < 0 ? 1.f : 0.f);
                break;
            case MIRROR:
                x = std::clamp(x, -1.0f, 2.0f);
                if (x < 0) x *= -1;
                if (x > 1) x = 2 - x;
                y = std::clamp(y, -1.0f, 2.0f);
                if (y < 0) y *= -1;
                if (y > 1) y = 2 - y;
                break;
            case FILL:
                return m_fillColor;
                break;
        }
    }

    // now sample from the texture appropriately
    switch (m_sampleMode) {
        case NEAREST: {
            int x_near = std::roundf(x * (m_texture.get_width() - 1));
            int y_near = std::roundf(y * (m_texture.get_height() - 1));
            RGB sample = m_texture(x_near, y_near);
            return glm::vec3(sample.r / 255.f, sample.g / 255.f, sample.b / 255.f);
        }
        case BILINEAR: {
            float x_sample = x * (m_texture.get_width() - 1);
            float y_sample = y * (m_texture.get_height() - 1);

            int x1 = std::floorf(x_sample);
            // minor correction needed for sampling at 1.0, as then the upper sample point is out of bounds
            x1 -= (x1 == m_texture.get_width() - 1) ? 1 : 0;
            int x2 = x1 + 1;

            int y1 = std::floorf(y_sample);
            // minor correction needed for sampling at 1.0, as then the upper sample point is out of bounds
            y1 -= (y1 == m_texture.get_height() - 1) ? 1 : 0;
            int y2 = y1 + 1;


            // sample texture at 4 corners
            const RGB c11 = m_texture(x1, y1);
            const RGB c12 = m_texture(x1, y2);
            const RGB c21 = m_texture(x2, y1);
            const RGB c22 = m_texture(x2, y2);

            // compute weights for bilinear interpolation
            float q11 = (x2 - x_sample) * (y2 - y_sample);
            float q12 = (x2 - x_sample) * (y_sample - y1);
            float q21 = (x_sample - x1) * (y2 - y_sample);
            float q22 = (x_sample - x1) * (y_sample - y1);

            assert(q11 + q12 + q21 + q22 != 0);

            RGB lerp{};
            lerp.r = std::roundf(c11.r * q11 + c12.r * q12 + c21.r * q21 + c22.r * q22);
            lerp.g = std::roundf(c11.g * q11 + c12.g * q12 + c21.g * q21 + c22.g * q22);
            lerp.b = std::roundf(c11.b * q11 + c12.b * q12 + c21.b * q21 + c22.b * q22);
            return glm::vec3(lerp.r / 255.f, lerp.g / 255.f, lerp.b / 255.f);
        }
    }
    return m_fillColor;
}

Sampler::Sampler(samplingMode sampling, wrappingMode wrapping)
{
    m_texture = Texture();
    m_sampleMode = sampling;
    m_wrapMode = wrapping;
}

Sampler::Sampler(const char* path, samplingMode sampling, wrappingMode wrapping)
{
    m_texture = Texture(path);
    m_sampleMode = sampling;
    m_wrapMode = wrapping;
}

Sampler::Sampler(Texture& texture, samplingMode sampling, wrappingMode wrapping) :
    m_sampleMode(sampling),
    m_wrapMode(wrapping),
    m_texture(texture)
{}