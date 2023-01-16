#include "tgaimage.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>

#include <iostream>
#include <vector>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

// Bresenham's line drawing algorithm (see https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm)
// Since Bresenham's algorithm in its base form is only defined over a single octant of lines (those 
// with slope between 0 and 1), we must transform the other cases into this octant.
void drawLine(int x0, int y0, int x1, int y1, TGAImage &img, const TGAColor &col) {
    // check for "steep" lines (slope > 1), if so simply draw transposally
    bool transpose = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        transpose = true;
    }
    // since line must be drawn in increasing order
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
	int dx = x1 - x0;
	int dy = y1 - y0;
    // cover slopes between 0 and -1 by tracking if y is increasing or decreasing
	int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int D = (2 * dy) - dx;
    int y = y0;
    for (int x = x0; x < x1; ++x) {
        if (transpose) {
            img.set(y, x, col);
        }
        else {
            img.set(x, y, col);
        }
        if (D > 0) {
            y += yi;
            D += (2 * (dy - dx));
        }
        else {
            D += (2 * dy);
        }
    }
}

int main(int argc, char** argv) {
    // Line drawing test
    TGAImage lines(50, 50, TGAImage::RGB);
    drawLine(10, 10, 30, 20, lines, red);
    drawLine(15, 15, 20, 40, lines, green);
    drawLine(20, 20, 10, 10, lines, blue);
    drawLine(0, 50, 10, 10, lines, red);
    drawLine(0, 10, 50, 10, lines, white);
    drawLine(40, 0, 40, 50, lines, white);
	lines.flip_vertically(); // so that origin (0,0) is bottom left, not top left
	lines.write_tga_file("linetest.tga");

    // Wireframe drawing test
    int width = 1000;
    int height = 1000;
    std::string modelPath = "Resources\\african_head.obj";

    TGAImage wireframe(width, height, TGAImage::RGB);
    Assimp::Importer importer;
    const aiScene* aScene = importer.ReadFile(modelPath,
        //post processing options
        aiProcess_Triangulate | // transform all model primitives into traingles if they aren't already
        aiProcess_FlipUVs | // flip texture coordinates on y-axis
        aiProcess_GenNormals | // creates normal vectors for each vertex if the model does not already have them
        aiProcess_OptimizeMeshes); // attempts to join multiple meshes into larger meshes to reduce number of drawing calls
    if (!aScene || aScene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !aScene->mRootNode) {
        std::cout << "Error loading scene: " << importer.GetErrorString() << std::endl;
        return -1;
    }

    for (int meshNum = 0; meshNum < aScene->mNumMeshes; meshNum++) {
        aiMesh* aMesh = aScene->mMeshes[meshNum];
        std::vector<glm::vec3> vertexPositions;
        std::vector<unsigned int> indices;

        // Extract vertex positions
        for (int i = 0; i < aMesh->mNumVertices; i++) {
            aiVector3D assimpVec = aMesh->mVertices[i];
            vertexPositions.push_back(glm::vec3(assimpVec.x, assimpVec.y, assimpVec.z));
        }

        // Iterate over each face and draw the lines forming the face
        for (unsigned int i = 0; i < aMesh->mNumFaces; i++) {
            aiFace face = aMesh->mFaces[i];
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
            }
        }
    }
    wireframe.flip_vertically(); // so that origin (0,0) is bottom left, not top left
    wireframe.write_tga_file("wireframe.tga");


	return 0;
}