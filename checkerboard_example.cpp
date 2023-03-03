#include "shaderProgram.h"
#include "renderer.h"
#include "sampler.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <stb_image_write.h>
#include "External/stb_image.h"
#include "examples.h"

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

		renderer.draw(program, vertices, indices, "Output/checkerboard_example.tga");

        if (!openGLComparison) return 0;
        return OpenGLRender(width, height, vertices, indices, projection, view);
	}

    int OpenGLRender(int width, int height, std::vector<Vertex>& vertices, std::vector<int>& indices, glm::mat4& projection, glm::mat4& view)
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
        //create the openGL vertex buffer and vertex array - also using element buffer object (index array)
        unsigned int vbo, vao, ebo;
        glGenBuffers(1, &vbo);
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &ebo);

        //bind the vertex array first
        glBindVertexArray(vao);
        //then bind vertex buffer object (vbo) to vertex buffer type target (GL_ARRAY_BUFFER), so now any calls on that configures the currently bound buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        //copy vertex data into the buffer
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        //GL_STREAM_DRAW as the data is both set and drawn only once

        // Element buffer object is just index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

        //tell OpenGL how to interpret the vertex data (per vertex attribute) and enable each attribute
        //arguments to glVertexAttribPointer are (index, size, type, normalised, stride, offset)
        //indexes defined using layouts in shaders, position is 0, texCoords is 1
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(1);

        // -------- SHADERS --------
        const char* vertexShaderSource = "#version 460 core\n"
            "layout (location = 0) in vec3 inPos;\n"
            "layout (location = 1) in vec2 inTexCoords;\n"
            "out vec2 texCoords;\n"
            "uniform mat4 MVP;"
            "void main()\n"
            "{\n"
            "    gl_Position = MVP * vec4(inPos, 1.0);\n"
            "    texCoords = inTexCoords;\n"
            "}\0";

        const char* fragmentShaderSource = "#version 460 core\n"
            "out vec4 FragColor;\n"
            "in vec2 texCoords;\n"
            "uniform sampler2D checkerboard;\n"
            "void main()\n"
            "{\n"
            "    FragColor = texture(checkerboard, texCoords);\n"
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

        // Checkerboard texture
        int imWidth, imHeight, imChannels;
        unsigned char* data = stbi_load("Resources/checkerboard.png", &imWidth, &imHeight, &imChannels, 3);
        unsigned int texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);


        // Assign model view projection matrix for use in vertex shader
        int MVPLocation = glGetUniformLocation(shaderProgram, "MVP");
        glm::mat4 MVP = projection * view;
        glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &MVP[0][0]);

        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(vao);
        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
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
        stbi_write_tga("Output/checkerboard_example_openGL.tga", width, height, nrChannels, buffer.data());

        glfwTerminate();

        return 0;
    }
}