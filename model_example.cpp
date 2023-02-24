#include "shaderProgram.h"
#include "renderer.h"
#include "sampler.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

// A more complex example which performs blinn-phong shading alongside diffuse, normal and AO mapping on a given model

namespace ModelExample {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;
	};

	struct Varying {
		glm::vec4 gl_Position; // required
		glm::vec2 texCoords; // for texture mapping
		glm::vec3 camPos_tangent; // for normal mapped lighting
		glm::vec3 lightDir_tangent; // for normal mapped lighting
		glm::vec3 worldPos_tangent; // for normal mapped lighting
	};

	struct SkullProgram : public IShaderProgram<Vertex, Varying> {

		glm::mat4 m_model;
		glm::mat4 m_view;
		glm::mat4 m_projection;
		glm::mat3 m_normalMatrix; // normal transformation must preserve orthogonality of normal vectors
		glm::vec3 m_camPos;
		Sampler m_diffuseSampler;
		Sampler m_normalSampler;
		Sampler m_specularSampler;
		Sampler m_AOSampler;

		// Lighting properties
		glm::vec3 m_lightDir = glm::normalize(glm::vec3(2, 2, 5)); // vec to light

		SkullProgram(glm::mat4 model, glm::mat4 view, glm::mat4 projection, glm::vec3 camPos,
			const char* diffusePath, const char* normalPath, const char* specularPath, const char* AOPath) :
			m_model(model),
			m_view(view),
			m_projection(projection),
			m_camPos(camPos),
			m_diffuseSampler(diffusePath, BILINEAR, CLAMPTOEDGE),
			m_normalSampler(normalPath, BILINEAR, CLAMPTOEDGE),
			m_specularSampler(specularPath, BILINEAR, CLAMPTOEDGE),
			m_AOSampler(AOPath, BILINEAR, CLAMPTOEDGE),
			m_normalMatrix(glm::transpose(glm::inverse(model)))
		{}

		virtual Varying vertexShader(const Vertex& input) {
			Varying ret{};
			ret.gl_Position = m_projection * m_view * m_model * glm::vec4(input.position, 1.f);
			ret.texCoords = input.texCoords;

			// for normal mapping, want to transform lighting vectors to tangent space
			glm::vec3 T = glm::normalize(glm::vec3(m_model * glm::vec4(input.tangent, 0.0)));
			glm::vec3 N = glm::normalize(glm::mat3(m_model) * input.normal);
			glm::vec3 B = glm::cross(N, T);

			glm::mat3 TBN = glm::transpose(glm::mat3(T, B, N));
			ret.lightDir_tangent = TBN * m_lightDir;
			ret.camPos_tangent = TBN * m_camPos;
			ret.worldPos_tangent = TBN * glm::vec3(m_model * glm::vec4(input.position, 1.f));

			return ret;
		}

		virtual glm::vec3 fragmentShader(const Varying& fragIn) {
			// Blinn-Phong shading
			glm::vec3 norm = m_normalSampler(fragIn.texCoords.x, fragIn.texCoords.y);
			norm = glm::normalize(norm * glm::vec3(2.0) - glm::vec3(1.0));

			// Diffuse
			float diff = std::max(glm::dot(fragIn.lightDir_tangent, norm), 0.0f);

			// Specular
			glm::vec3 viewDir = glm::normalize(fragIn.camPos_tangent - fragIn.worldPos_tangent); // vec to viewer
			glm::vec3 H = glm::normalize(viewDir + fragIn.lightDir_tangent);
			float spec = std::pow(std::max(glm::dot(norm, H), 0.0f), 16.0f) * 0.5f;

			return (diff * m_diffuseSampler(fragIn.texCoords.x, fragIn.texCoords.y)
				+ spec * (glm::vec3(1.0) - m_specularSampler(fragIn.texCoords.x, fragIn.texCoords.y)))
				* m_AOSampler(fragIn.texCoords.x, fragIn.texCoords.y);
			//return m_specularSampler(fragIn.texCoords.x, fragIn.texCoords.y);
		}

		virtual Varying interpolate(const Varying& a, const Varying& b, const Varying& c, float ba, float bb, float bc) {
			Varying ret{};
			ret.texCoords = ba * a.texCoords + bb * b.texCoords + bc * c.texCoords;
			ret.camPos_tangent = ba * a.camPos_tangent + bb * b.camPos_tangent + bc * c.camPos_tangent;
			ret.lightDir_tangent = ba * a.lightDir_tangent + bb * b.lightDir_tangent + bc * c.lightDir_tangent;
			ret.worldPos_tangent = ba * a.worldPos_tangent + bb * b.worldPos_tangent + bc * c.worldPos_tangent;
			return ret;
		}
	};

	int run() {
		int width = 1920;
		int height = 1080;

		glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
		glm::vec3 camPos = glm::vec3(0, 10, 20);
		glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0, 2.5, 0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 projection = glm::perspective(glm::radians(60.f), (float)width / height, 0.1f, 100.0f);

		SkullProgram program(
			model, view, projection, camPos,
			"Resources/demon-skull/textures/DemonSkull_Diffuse.png",
			"Resources/demon-skull/textures/DemonSkull_Normal.png",
			"Resources/demon-skull/textures/DemonSkull_Roughness.png",
			"Resources/demon-skull/textures/DemonSkull_AO.png"
		);
		Renderer<Vertex, Varying> renderer(width, height);

		// set up vertex buffer and index buffer (using ASSIMP)
		std::vector<Vertex> vertices;
		std::vector<int> indices;

		//load .obj model file into Assimp's scene object, from which we then extract the necessary data we need
		Assimp::Importer importer;
		const aiScene* aScene = importer.ReadFile("Resources/demon-skull/source/DemonSkull_Optimized2.fbx",
			//post processing options
			aiProcess_Triangulate | // transform all model primitives into traingles if they aren't already
			aiProcess_GenNormals | // creates normal vectors for each vertex if the model does not already have them
			aiProcess_OptimizeMeshes | // attempts to join multiple meshes into larger meshes to reduce number of drawing calls
			aiProcess_CalcTangentSpace); // compute tangent vectors for the loaded vertices
		if (!aScene || aScene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !aScene->mRootNode) {
			std::cout << "Error loading scene: " << importer.GetErrorString() << std::endl;
			return -5;
		}

		// Reserve necessary space for the vertex and index vectors
		int totalVertices = 0, totalIndices = 0;
		for (int i = 0; i < aScene->mNumMeshes; i++) {
			totalVertices += aScene->mMeshes[i]->mNumVertices;
			totalIndices += (aScene->mMeshes[i]->mNumFaces * 3);
		}
		vertices.reserve(totalVertices);
		indices.reserve(totalIndices);

		// Extract vertex information as required, as well as populating Index buffer
		for (int i = 0; i < aScene->mNumMeshes; i++) {
			aiMesh* mesh = aScene->mMeshes[i];
			// Vertices
			for (int j = 0; j < mesh->mNumVertices; j++) {
				Vertex v{};

				// position
				aiVector3D assimpVec = mesh->mVertices[j];
				v.position = glm::vec3(assimpVec.x, assimpVec.y, assimpVec.z);

				// normal
				assimpVec = mesh->mNormals[j];
				v.normal = glm::vec3(assimpVec.x, assimpVec.y, assimpVec.z);

				// TexCoords
				assimpVec = mesh->mTextureCoords[0][j];
				v.texCoords = glm::vec2(assimpVec.x, assimpVec.y);

				// tangent
				assimpVec = mesh->mTangents[j];
				v.tangent = glm::vec3(assimpVec.x, assimpVec.y, assimpVec.z);

				vertices.push_back(v);
			}

			// Indices
			for (int j = 0; j < mesh->mNumFaces; j++) {
				aiFace face = mesh->mFaces[j];
				for (unsigned int k = 0; k < face.mNumIndices; k++) {
					indices.push_back(face.mIndices[k]);
				}
			}
		}

		renderer.draw(program, vertices, indices);

		return 0;
	}
}