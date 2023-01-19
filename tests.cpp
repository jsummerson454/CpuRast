#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"

#include "tgaimage.h"
#include "rasterizer.h"
#include "tests.h"
#include "vertex_processing.h"

#include <iostream>
#include <vector>

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
    float* zbuffer = new float[height*width];
    for (int i = 0; i < height * width; i++) {
        zbuffer[i] = 2.0;
    }
    std::string modelPath = "Resources\\african_head.obj";

    TGAImage wireframe(width, height, TGAImage::RGB);
    TGAImage triangular(width, height, TGAImage::RGB);

    glm::mat4 view = glm::lookAt(glm::vec3(50, 50, 100), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 projection = createProjectionMatrix(glm::radians(70.0), (float)width / height, 0.1, 1000.0);

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
            ipoint2d triangle[3];
            float triangleDepths[3];
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
                // we also require the depth, so convert it from the [-1, 1] range to [0, 1] for now
                // TODO - scale values to fit the buffer size as fixed point, i.e. values in range [0,255] for uint 8 buffer 
                // (see https://www.khronos.org/opengl/wiki/Depth_Buffer_Precision)
                triangleDepths[j] = (v0.z + 1.0)/2.0;

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

    delete zbuffer;
}