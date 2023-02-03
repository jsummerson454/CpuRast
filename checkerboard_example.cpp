#include "shaderProgram.h"
#include "renderer.h"
#include "sampler.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace CheckerboardExample {
	struct Vertex {
		glm::vec3 pos;
		glm::vec2 texCoords;
	};

	struct Varying {
		glm::vec4 gl_Position;
		glm::vec2 texCoords;
	};

	struct TextureProgram : public IShaderProgram<Vertex, Varying> {

		glm::mat4 m_view;
		glm::mat4 m_projection;
		Sampler m_sampler;

		TextureProgram(glm::mat4 view, glm::mat4 projection, const char* texturePath) :
			m_view(view),
			m_projection(projection),
			m_sampler(texturePath, BILINEAR, MIRROR)
		{}

		virtual Varying vertexShader(const Vertex& input) {
			Varying ret{};
			ret.gl_Position = m_projection * m_view * glm::vec4(input.pos, 1.f);
			ret.texCoords = input.texCoords;
			return ret;
		}

		virtual glm::vec3 fragmentShader(const Varying& interpolatedInput) {
			return m_sampler(interpolatedInput.texCoords.x, interpolatedInput.texCoords.y);
		}

		virtual Varying interpolate(const Varying& a, const Varying& b, const Varying& c, float ba, float bb, float bc) {
			Varying ret{};
			ret.texCoords = ba * a.texCoords + bb * b.texCoords + bc * c.texCoords;
			return ret;
		}
	};

	int run(bool openGLComparison) {
		int width = 750;
		int height = 750;

		glm::mat4 view = glm::lookAt(glm::vec3(0.0, 5.0, 7.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 projection = glm::perspective(glm::radians(90.f), (float)width / height, 0.1f, 100.0f);

		TextureProgram program(view, projection, "Resources/checkerboard.png");
		Renderer<Vertex, Varying> renderer(width, height);

		// set up vertex buffer and index buffer
		std::vector<Vertex> vertices;
		// Plane vertices
		vertices.push_back(Vertex{ glm::vec3(-5, 0, -20), glm::vec2(0, 0) });
		vertices.push_back(Vertex{ glm::vec3(5, 0, -20), glm::vec2(1, 0) });
		vertices.push_back(Vertex{ glm::vec3(-5, 0, 5), glm::vec2(0, 1) });
		vertices.push_back(Vertex{ glm::vec3(5, 0, 5), glm::vec2(1, 1) });

		// Two triangles forming the plane quad
		std::vector<int> indices{ 0, 2, 1, 1, 2, 3 };

		renderer.draw(program, vertices, indices);

		return 0;
	}
}