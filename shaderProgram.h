#pragma once
#include <glm/glm.hpp>

template <typename Vertex, typename Varying>
struct IShaderProgram {
	virtual Varying vertexShader(const Vertex& input) = 0 ;
	virtual glm::vec3 fragmentShader(const Varying& interpolatedInput) = 0;
	virtual Varying interpolate(const Varying& a, const Varying& b, const Varying& c, float ba, float bb, float bc) = 0;
	template <typename Vertex, typename Varying>
	friend class Renderer;
};