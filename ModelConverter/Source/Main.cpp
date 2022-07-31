
#define PMDL_IMPLEMENTATION
#include "PMDL.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <meshoptimizer.h>

#include <vector>

constexpr float LOD_INDEX_TARGET[3] =
{
	0.5f, 0.2f, 0.1f
};

class Model
{
public:
	

	std::vector<pmdl::Mesh> meshes;
	std::vector<pmdl::Mesh> lods;
	std::vector<pmdl::Material1> materials;

	std::vector<pmdl::Vertex> vertices;
	std::vector<uint32_t> indices;

	std::string dir;


	void LoadFromFile(const std::string& filepath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			printf("Assimp Error: %s", importer.GetErrorString());
			return;
		}
		dir = filepath;

		processNode(scene->mRootNode, scene);

		for (uint32_t i = 0; i < scene->mNumMaterials; i++)
		{
			materials.push_back(ProcessMaterials(scene->mMaterials[i]));
		}
	}

	void processNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			pmdl::Mesh m = processMesh(mesh, scene);

			meshes.push_back(m);
		}

		
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	pmdl::Material1 ProcessMaterials(aiMaterial* material)
	{
		pmdl::Material1 mat;

		aiColor3D color(0.f, 0.f, 0.f);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);

		mat.albedo = { color.r, color.g, color.b, 1.0f };

		mat.roughness = 0.5f;
		mat.metallic = 0.5f;

		return mat;
	}

	pmdl::Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		uint32_t firstIndex = indices.size();
		uint32_t indexCount = 0;

		uint32_t materialIndex = mesh->mMaterialIndex;

		uint32_t vertexOffset = vertices.size();

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			pmdl::Vertex vertex;
			// process vertex positions, normals and texture coordinates

			vertex.x = mesh->mVertices[i].x;
			vertex.y = mesh->mVertices[i].y;
			vertex.z = mesh->mVertices[i].z;

			vertex.nx = mesh->mNormals[i].x;
			vertex.ny = mesh->mNormals[i].y;
			vertex.nz = mesh->mNormals[i].z;

			vertex.tx = mesh->mTangents[i].x;
			vertex.ty = mesh->mTangents[i].y;
			vertex.tz = mesh->mTangents[i].z;

			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				vertex.u = mesh->mTextureCoords[0][i].x;
				vertex.v = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.u = 0.0;
				vertex.v = 0.0f;
			}
				


			vertices.push_back(vertex);
		}
		
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{ 
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		indexCount = indices.size() - firstIndex;


		return { firstIndex, indexCount, materialIndex, vertexOffset };
	}


};

struct
{
	const char* baseModelPath;
	const char* outputPath;
} arguments;

void ParseArgs(int count, char* argv[])
{
	// Only the name is present
	if (count == 1)
		return;
	
	int parseCount = 1;

	if (count > 1)
	{
		arguments.baseModelPath = argv[parseCount];
		printf("Base Model Path: %s\n", argv[parseCount]);
		parseCount++;
	}

	if (parseCount < count)
	{
		arguments.outputPath = argv[parseCount];
		printf("Output Path: %s\n", argv[parseCount]);
	}

}

int main(int argc, char* argv[])
{
	ParseArgs(argc, argv);

	Model model;


	// use assimp to load the model
	{
		model.LoadFromFile(arguments.baseModelPath);

		printf("Loaded Model\n");

		printf("Writing Model:\n ");

		pmdl::Header1 header{};
		pmdl::InitHeader(&header);
		pmdl::InitHeaderOffsets1(&header, model.vertices.size(), model.indices.size(),
			model.meshes.size(), model.materials.size(), 0, model.meshes.size());

		FILE* file = fopen(arguments.outputPath, "wb");

		pmdl::WriteHeader1(file, &header);

		pmdl::WriteVertices(file, &header, model.vertices.data(), model.vertices.size());

		printf("Number of Vertices Written: %d\n", model.vertices.size());

		pmdl::WriteIndices(file, &header, model.indices.data(), model.indices.size());

		printf("Number of Indices Written: %d\n", model.indices.size());

		for (uint32_t i = 0; i < model.meshes.size(); i++)
			pmdl::WriteMesh1(i, model.meshes[i], &header, file);

		for (uint32_t i = 0; i < model.lods.size(); i++)
			pmdl::WriteLOD1(file, &header, model.lods[i], i);

		printf("Number of Meshes Written: %d\n", model.meshes.size());
		printf("Number of LOD Meshes Written: %d\n", model.lods.size());

		for (uint32_t i = 0; i < model.materials.size(); i++)
			pmdl::WriteMaterial1(file, &header, &model.materials[i], i);

		printf("Number of Materials Written: %d\n", model.materials.size());


		fclose(file);
	}


	/* {
		pmdl::Header1 header{};
		pmdl::InitHeader(&header);
		pmdl::InitHeaderOffsets1(&header, 4, 6, 1, 1, 0);

		pmdl::Mesh mesh1;
		mesh1.firstIndex = 0;
		mesh1.indexCount = 6;

		pmdl::Material1 material{};
		material.albedo = { 1.0f, 0.0f, 0.0f, 1.0f };


		FILE* file = fopen("TestModel.pmdl", "wb");

		pmdl::WriteHeader1(file, &header);

		pmdl::WriteMesh1(0, mesh1, &header, file);

		pmdl::WriteMaterial1(file, &header, &material, 0);

		fclose(file);
	}*/

	/*{
		pmdl::Header1 header{};

		FILE* file = fopen("TestModel.pmdl", "rb");

		header = pmdl::ReadHeader1(file);

		printf("Mesh Count: %d\n", header.meshCount);
		printf("Material Count: %d\n", header.materialCount);

		pmdl::Mesh mesh1 = pmdl::ReadMesh1(file, &header, 0);
		pmdl::Material1 material = pmdl::ReadMaterial1(file, &header, 0);

		printf("Albedo: %.3f, %.3f, %.3f, %.3f", material.albedo.x, material.albedo.y, material.albedo.z, material.albedo.w);


		fclose(file);
	}*/
}