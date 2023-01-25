#include "shaderProgram.h"
#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// User must define: Vertex struct, Varying struct, ShaderProgram (containing vertexShader, fragmentShader and interpolation functions)
struct Vertex {
	glm::vec3 pos;
	glm::vec3 col;
};

// IMPORTANT: Varying struct MUST contain glm::vec4 gl_Position member, which has the result of the MVP transform written to in vertex shader
struct Varying {
	glm::vec4 gl_Position;
	glm::vec3 col;
};

struct ColourProgram : public IShaderProgram<Vertex, Varying> {
	// TODO: incorporate uniforms into the shader program....
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 6.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 projection = glm::perspective(glm::radians(60.f), (float)1280/720, 0.1f, 100.0f);

	virtual Varying vertexShader(const Vertex& input) {
		Varying ret{};
		ret.gl_Position = projection * view * glm::vec4(input.pos, 1.f);
		ret.col = input.col;
		return ret;
	}

	virtual glm::vec3 fragmentShader(const Varying& interpolatedInput) {
		return interpolatedInput.col;
	}

	virtual Varying interpolate(const Varying& a, const Varying& b, const Varying& c, float ba, float bb, float bc) {
		Varying ret{};
		ret.col = ba * a.col + bb * b.col + bc * c.col;
		return ret;
	}
};

int main(int argc, char** argv) {
	int width = 1280;
	int height = 720;

	ColourProgram program;
	Renderer<Vertex, Varying> renderer(width, height);

	// set up vertex buffer and index buffer
	std::vector<Vertex> vertices;
	// First triangle
	vertices.push_back(Vertex{ glm::vec3(-3, -2, 0), glm::vec3(1, 0, 0) });
	vertices.push_back(Vertex{ glm::vec3(1, 0, 1.5), glm::vec3(0, 1, 0) });
	vertices.push_back(Vertex{ glm::vec3(-2, 2, 0), glm::vec3(0, 0, 1) });

	// Second triangle
	vertices.push_back(Vertex{ glm::vec3(4, -1, -1), glm::vec3(1, 1, 0) });
	vertices.push_back(Vertex{ glm::vec3(4, 3, -1), glm::vec3(0, 1, 1) });
	vertices.push_back(Vertex{ glm::vec3(-2, 0, 2), glm::vec3(1, 0, 1) });

	std::vector<int> indices{ 0, 1, 2, 3, 4, 5 };

	renderer.draw(program, vertices, indices);

	return 0;
}