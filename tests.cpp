#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/string_cast.hpp>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "tgaimage.h"
#include "rasterizer.h"
#include "tests.h"
#include "vertex_processing.h"

#include <iostream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

// Line drawing test
void lineTest() {
	TGAImage lines(50, 50, TGAImage::RGB);
	drawLine(10, 10, 30, 20, lines, red);
	drawLine(15, 15, 20, 40, lines, green);
	drawLine(20, 20, 10, 10, lines, blue);
	drawLine(0, 50, 10, 10, lines, red);
	drawLine(0, 10, 50, 10, lines, white);
	drawLine(40, 0, 40, 50, lines, white);
	lines.flip_vertically(); // so that origin (0,0) is bottom left, not top left
	lines.write_tga_file("linetest.tga");
}

// Model drawing test (wireframe + random colours, perspective flag controls whether perspective projection is actually applied or not)
void faceModelTest(bool perspective) {
    int width = 1280;
    int height = 720;
    zbuffer_t* zbuffer = new zbuffer_t[height*width];
    for (int i = 0; i < height * width; i++) {
        zbuffer[i] = ZBUFFSCALE;
    }
    std::string modelPath = "Resources\\african_head.obj";

    TGAImage wireframe(width, height, TGAImage::RGB);
    TGAImage triangular(width, height, TGAImage::RGB);

    glm::mat4 view = glm::lookAt(glm::vec3(1, 1, 3), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 projection = createProjectionMatrix(40.0, (float)width / height, 0.1, 100.0);

    Assimp::Importer importer;
    const aiScene* aScene = importer.ReadFile(modelPath,
        //post processing options
        aiProcess_Triangulate | // transform all model primitives into traingles if they aren't already
        aiProcess_FlipUVs | // flip texture coordinates on y-axis
        aiProcess_GenNormals | // creates normal vectors for each vertex if the model does not already have them
        aiProcess_OptimizeMeshes); // attempts to join multiple meshes into larger meshes to reduce number of drawing calls
    if (!aScene || aScene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !aScene->mRootNode) {
        std::cout << "Error loading model: " << importer.GetErrorString() << std::endl;
        return;
    }

    for (int meshNum = 0; meshNum < aScene->mNumMeshes; meshNum++) {
        aiMesh* aMesh = aScene->mMeshes[meshNum];
        std::vector<glm::vec3> vertexPositions;

        // Extract vertex positions
        vertexPositions.reserve(aMesh->mNumVertices);
        for (int i = 0; i < aMesh->mNumVertices; i++) {
            aiVector3D assimpVec = aMesh->mVertices[i];
            glm::vec3 vertex = glm::vec3(assimpVec.x, assimpVec.y, assimpVec.z);
            // apply MVP and perspective divide, end up with NDC coordinates
            if (perspective) {
                vertex = perspectiveDivide(projection * view * glm::vec4(vertex, 1.0));
            }
            vertexPositions.push_back(vertex);
        }


        // Iterate over each face and draw the lines forming the face into wireframe image, and
        // the triangles themselves into the triangular image
        for (unsigned int i = 0; i < aMesh->mNumFaces; i++) {
            aiFace face = aMesh->mFaces[i];
            ipoint2d triangle[3] = {};
            zbuffer_t triangleDepths[3] = {};
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                int i0 = face.mIndices[j];
                int i1 = face.mIndices[(j + 1) % face.mNumIndices];

                glm::vec3 v0 = vertexPositions[i0];
                glm::vec3 v1 = vertexPositions[i1];

                // converting to screenspace (from NDC if perspective enabled, or directly from model
                // coords which in range -1 to 1)
                int x0 = (v0.x + 1.) * width / 2.;
                int y0 = (v0.y + 1.) * height / 2.;
                int x1 = (v1.x + 1.) * width / 2.;
                int y1 = (v1.y + 1.) * height / 2.;

                drawLine(x0, y0, x1, y1, wireframe, white);

                triangle[j] = ipoint2d{ x0, y0 };
                // ASSUMING CLIPPED AND CULLED NEAR/FAR PLANES AT THIS POINT OF PIPELINE, hence no need for
                // checking boundary conditions on NDC coordinates
                // we also require the depth, so convert it from the [-1, 1] range to [0, 1] for now
                // (see https://www.khronos.org/opengl/wiki/Depth_Buffer_Precision)
                triangleDepths[j] = (zbuffer_t) std::roundf((0.5f * v0.z + 0.5f) * ZBUFFSCALE);
                std::cout << v0.z << std::endl;
                std::cout << (int)triangleDepths[j] << std::endl << std::endl;
            }

            TGAColor randomColor = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);
            drawTriangle(triangle[0], triangle[1], triangle[2],
                         triangleDepths[0], triangleDepths[1], triangleDepths[2],
                         zbuffer, triangular, randomColor);
        }
    }
    wireframe.flip_vertically(); // so that origin (0,0) is bottom left, not top left
    wireframe.write_tga_file("wireframe.tga");
    triangular.flip_vertically();
    triangular.write_tga_file("triangular.tga");

    delete[] zbuffer;
}

struct Vertex {
    glm::vec3 pos;
    glm::vec3 col;
};

void zBufferTest(bool openGLComparison) {
    int width = 1280;
    int height = 720;
    zbuffer_t* zbuffer = new zbuffer_t[height * width];
    for (int i = 0; i < height * width; i++) {
        zbuffer[i] = ZBUFFSCALE;
    }

    TGAImage image(width, height, TGAImage::RGB);

    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 6.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 projection = createProjectionMatrix(60.0, (float)width / height, 0.1, 100.0);

    std::vector<Vertex> vertices;
    // First triangle
    vertices.push_back(Vertex{ glm::vec3(-3, -2, 0), glm::vec3(255, 0, 0) });
    vertices.push_back(Vertex{ glm::vec3(1, 0, 1.5), glm::vec3(0, 255, 0) });
    vertices.push_back(Vertex{ glm::vec3(-2, 2, 0), glm::vec3(0, 0, 255) });

    // Second triangle
    vertices.push_back(Vertex{ glm::vec3(4, -1, -1), glm::vec3(255, 255, 0) });
    vertices.push_back(Vertex{ glm::vec3(4, 3, -1), glm::vec3(0, 255, 255) });
    vertices.push_back(Vertex{ glm::vec3(-2, 0, 2), glm::vec3(255, 0, 255) });

    // apply MVP and perspective divide to each vertex:
    for (int i = 0; i < vertices.size(); i++) {
        vertices[i].pos = perspectiveDivide(projection * view * glm::vec4(vertices[i].pos, 1.0));
    }

    // iterate over each triangle and rasterize it
    for (int i = 0; i < 6; i += 3) {
        rasterPoint triangle[3] = {};
        for (int j = 0; j < 3; j++) {
            Vertex v = vertices[i + j];
            rasterPoint point;
            point.col = v.col;
            // viewport transform, map [-1, 1] NDC coordinates into screenspace coordinates
            point.pos = ipoint2d {
                (int)((v.pos.x + 1.) * width / 2.),
                (int)((v.pos.y + 1.) * height / 2.)
            };
            // map [-1, 1] NDC depth into range [0,1]
            point.z = (v.pos.z + 1.f) / 2.f;
       
            triangle[j] = point;
        }
        rasterTriangle(triangle[0], triangle[1], triangle[2], zbuffer, image);
    }
    image.flip_vertically(); // so that origin (0,0) is bottom left, not top left
    image.write_tga_file("zBufferTest.tga");

    delete[] zbuffer;



    // OPENGL COMPARISON RENDER - openGL complains about rendering without device context, which is
    // typically attached to a window, so we create a window anyway despite rendering to an image file
    if (!openGLComparison) return;

    // first reset the vertices array, since we modified it previously
    vertices = {};
    // First triangle
    vertices.push_back(Vertex{ glm::vec3(-3, -2, 0), glm::vec3(255, 0, 0) });
    vertices.push_back(Vertex{ glm::vec3(1, 0, 1.5), glm::vec3(0, 255, 0) });
    vertices.push_back(Vertex{ glm::vec3(-2, 2, 0), glm::vec3(0, 0, 255) });

    // Second triangle
    vertices.push_back(Vertex{ glm::vec3(4, -1, -1), glm::vec3(255, 255, 0) });
    vertices.push_back(Vertex{ glm::vec3(4, 3, -1), glm::vec3(0, 255, 255) });
    vertices.push_back(Vertex{ glm::vec3(-2, 0, 2), glm::vec3(255, 0, 255) });


    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, "OpenGLWindow", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);

    // initialise glad before tying to use any openGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
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
        "    FragColor = vec4(col/255, 1.0f);\n"
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
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Error compiling fragment shader:\n" << infoLog << std::endl;
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
    stbi_write_tga("zBufferTestOpenGL.tga", width, height, nrChannels, buffer.data());

    glfwTerminate();
}