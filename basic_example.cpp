#include "shaderProgram.h"
#include "renderer.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "examples.h"

// A minimal example to showcase how the rendering framework works
// User must define themselves: Vertex struct, Varying struct, ShaderProgram (complete with
// vertex shader, fragment shader and interpolate functions), and provide the Vertex and Index
// buffers for drawing

namespace BasicExample {
	// Vertex struct
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 col;
	};

	// IMPORTANT: Varying struct MUST contain glm::vec4 gl_Position member, which has the result of the MVP transform written to in vertex shader
	struct Varying {
		glm::vec4 gl_Position;
		glm::vec3 col;
	};

	// Example ShaderProgram that only interpolates vertex colour attributes
	struct ColourProgram : public IShaderProgram<Vertex, Varying> {
		// Uniforms can simply be provided as member fields of the ShaderProgram, with setters if needed
		glm::mat4 m_view;// = glm::lookAt(glm::vec3(0, 0, 6.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 m_projection;// = glm::perspective(glm::radians(60.f), (float)1280 / 720, 0.1f, 100.0f);

		ColourProgram(glm::mat4 view, glm::mat4 projection) {
			m_view = view;
			m_projection = projection;
		}

		virtual Varying vertexShader(const Vertex& input) {
			Varying ret{};
			ret.gl_Position = m_projection * m_view * glm::vec4(input.pos, 1.f);
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

	int run(bool openGLComparison) {
		int width = 1080;
		int height = 720;

		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 6.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 projection = glm::perspective(glm::radians(60.f), (float)1280 / 720, 0.1f, 100.0f);

		ColourProgram program(view, projection);
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

		renderer.draw(program, vertices, indices, "Output/basic_example.tga");

		if (!openGLComparison) return 0;
        return OpenGLRender(width, height, vertices, projection, view);
	}

    int OpenGLRender(int width, int height, std::vector<Vertex>& vertices, glm::mat4& projection, glm::mat4& view)
    {
        // OPENGL COMPARISON RENDER - openGL complains about rendering without device context, which is
        // typically attached to a window, so we create a window anyway despite rendering to an image file
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        GLFWwindow* window = glfwCreateWindow(width, height, "OpenGLWindow", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -3;
        }
        glfwMakeContextCurrent(window);

        // initialise glad before tying to use any openGL functions
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -4;
        }

        glViewport(0, 0, width, height);

        // -------- VERTEX BUFFER AND ARRAY OBJECTS -------- 
        //create the openGL vertex buffer and vertex array
        unsigned int vbo, vao;
        glGenBuffers(1, &vbo);
        glGenVertexArrays(1, &vao);

        //bind the vertex array first
        glBindVertexArray(vao);
        //then bind vertex buffer object (vbo) to vertex buffer type target (GL_ARRAY_BUFFER), so now any calls on that configures the currently bound buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        //copy vertex data into the buffer
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        //GL_STREAM_DRAW as the data is both set and drawn only once

        //tell OpenGL how to interpret the vertex data (per vertex attribute) and enable each attribute
        //arguments to glVertexAttribPointer are (index, size, type, normalised, stride, offset)
        //indexes defined using layouts in shaders, position is 0, colour is 1
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, col));
        glEnableVertexAttribArray(1);

        // -------- SHADERS --------
        const char* vertexShaderSource = "#version 460 core\n"
            "layout (location = 0) in vec3 inPos;\n"
            "layout (location = 1) in vec3 inCol;\n"
            "out vec3 col;\n"
            "uniform mat4 MVP;"
            "void main()\n"
            "{\n"
            "    gl_Position = MVP * vec4(inPos, 1.0);\n"
            "    col = inCol;\n"
            "}\0";

        const char* fragmentShaderSource = "#version 460 core\n"
            "out vec4 FragColor;\n"
            "in vec3 col;\n"
            "void main()\n"
            "{\n"
            "    FragColor = vec4(col, 1.0f);\n"
            "}\0";

        unsigned int vertexShader, fragmentShader;
        int success;
        char infoLog[512];

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "Error compiling vertex shader:\n" << infoLog << std::endl;
            return -5;
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "Error compiling fragment shader:\n" << infoLog << std::endl;
            return -5;
        }

        unsigned int shaderProgram;
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Assign model view projection matrix for use in vertex shader
        int MVPLocation = glGetUniformLocation(shaderProgram, "MVP");
        glm::mat4 MVP = projection * view;
        glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &MVP[0][0]);

        glEnable(GL_DEPTH_TEST);

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glfwPollEvents();
            glfwSwapBuffers(window);
        }

        GLsizei nrChannels = 3;
        GLsizei stride = nrChannels * width;
        GLsizei bufferSize = stride * height;
        std::vector<char> buffer(bufferSize);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
        stbi_flip_vertically_on_write(true);
        stbi_write_tga("Output/basic_example_openGL.tga", width, height, nrChannels, buffer.data());

        glfwTerminate();

        return 0;
    }
}