#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "tgaimage.h"
#include "rasterizer.h"
#include "tests.h"

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

// Wireframe drawing test
void orthogonalFaceModelTest() {
    int width = 1000;
    int height = 1000;
    std::string modelPath = "Resources\\african_head.obj";

    TGAImage wireframe(width, height, TGAImage::RGB);
    TGAImage triangular(width, height, TGAImage::RGB);

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
            vertexPositions.push_back(glm::vec3(assimpVec.x, assimpVec.y, assimpVec.z));
        }

        // Iterate over each face and draw the lines forming the face into wireframe image, and
        // the triangles themselves into the triangular image
        for (unsigned int i = 0; i < aMesh->mNumFaces; i++) {
            aiFace face = aMesh->mFaces[i];
            glm::ivec2 triangle[3];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                int i0 = face.mIndices[j];
                int i1 = face.mIndices[(j + 1) % face.mNumIndices];

                glm::vec3 v0 = vertexPositions[i0];
                glm::vec3 v1 = vertexPositions[i1];

                int x0 = (v0.x + 1.) * width / 2.;
                int y0 = (v0.y + 1.) * height / 2.;
                int x1 = (v1.x + 1.) * width / 2.;
                int y1 = (v1.y + 1.) * height / 2.;

                drawLine(x0, y0, x1, y1, wireframe, white);

                triangle[j] = glm::ivec2(x0, y0);
            }

            TGAColor randomColor = TGAColor(rand() % 255, rand() % 255, rand() % 255, 255);
            drawTriangle(triangle[0], triangle[1], triangle[2], triangular, randomColor);
        }
    }
    wireframe.flip_vertically(); // so that origin (0,0) is bottom left, not top left
    wireframe.write_tga_file("wireframe.tga");
    triangular.flip_vertically();
    triangular.write_tga_file("triangular.tga");
}

// Triangle Drawing Test
void triangleTest() {
    int width = 300;
    int height = 100;
    TGAImage triangles(width, height, TGAImage::RGB);
    drawTriangle(glm::ivec2(-20, -20), glm::ivec2(50, -20), glm::ivec2(-20, 50), triangles, green);
    drawTriangle(glm::ivec2(0, 0), glm::ivec2(30, 10), glm::ivec2(10, 30), triangles, red);
    drawTriangle(glm::ivec2(250, 10), glm::ivec2(140, 50), glm::ivec2(160, 5), triangles, white);
    drawTriangle(glm::ivec2(170, 50), glm::ivec2(140, 50), glm::ivec2(250, 10), triangles, blue);

    triangles.flip_vertically(); // so that origin (0,0) is bottom left, not top left
    triangles.write_tga_file("triangles.tga");
}