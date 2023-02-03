#pragma once
#include "texture.h"
#include <glm/glm.hpp>

enum samplingMode {NEAREST, BILINEAR};
enum wrappingMode {CLAMPTOEDGE, REPEAT, MIRROR, FILL};

class Sampler {
private:
	samplingMode m_sampleMode;
	wrappingMode m_wrapMode;
	Texture m_texture;
	glm::vec3 m_fillColor = glm::vec3(0);
public:
	const glm::vec3 operator()(float x, float y);
	Sampler(samplingMode sampling = NEAREST, wrappingMode wrapping = CLAMPTOEDGE);
	Sampler(const char* path, samplingMode sampling = NEAREST, wrappingMode wrapping = CLAMPTOEDGE);
	Sampler(Texture& texture, samplingMode sampling = NEAREST, wrappingMode wrapping = CLAMPTOEDGE);
	void setSamplingMode(samplingMode mode) { m_sampleMode = mode; }
	void setWrappingMode(wrappingMode mode) { m_wrapMode = mode; }
	void setFillColor(glm::vec3 col) { m_fillColor = col; }
	// disable copy constructor, assignment operator and default constructor
	Sampler(const Sampler&) = delete;
	Sampler() = delete;
	Sampler& operator=(const Sampler&) = delete;
};