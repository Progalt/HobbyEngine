
#include "Model.h"
#include "../Renderer/RenderManager.h"
#include "../Renderer/Mesh.h"
#include "../Model/PMDL.h"
#include "../Core/Log.h"

void Model::Discard()
{
	mesh->Destroy();
}

void Model::LoadFromFile(const std::string& path, RenderManager* renderManager)
{
	// Currently PMDL doesn't have a sort of global loadModel function
	// Each section must be loaded as needed

	// Its quite fast to load and convert to our format because most it is fairly simple

	FILE* file = fopen(path.c_str(), "rb");

	pmdl::Header1 header = pmdl::ReadHeader1(file);

	pmdl::Vertex* vertices = pmdl::ReadVertices(file, &header);
	uint32_t* indices = pmdl::ReadIndices32bit(file, &header);

	mesh = renderManager->NewMesh();

	mesh->positions.resize(header.vertexCount);
	mesh->texCoords.resize(header.vertexCount);
	mesh->normals.resize(header.vertexCount);

	// Add all the vertex and index data to the mesh
	for (uint32_t i = 0; i < header.vertexCount; i++)
	{
		// PMDL outputs a vertex struct currently
		// so convert to our system
		mesh->positions[i] = { vertices[i].x, vertices[i].y, vertices[i].z };
		mesh->texCoords[i] = { vertices[i].u, vertices[i].v };
		mesh->normals[i] = { vertices[i].nx, vertices[i].ny, vertices[i].nz };
	}

	mesh->indices.resize(header.indexCount);

	for (uint32_t i = 0; i < header.indexCount; i++)
		this->mesh->indices[i] = indices[i];

	PMDL_FREE(vertices);
	PMDL_FREE(indices);

	this->mesh->GenerateMesh();

	for (uint16_t i = 0; i < header.meshCount; i++)
	{
		pmdl::Mesh mesh = pmdl::ReadMesh1(file, &header, i);

		for (uint16_t l = mesh.firstLOD; l < mesh.firstLOD + mesh.LODCount; l++)
			LODs.push_back(pmdl::ReadLOD1(file, &header, l));

		submeshes.push_back(mesh);
	}

	std::vector<Image> textures(header.textureCount);

	std::string directory = FileSystem::GetDirectory(path);


	for (uint16_t i = 0; i < header.textureCount; i++)
	{
		pmdl::Texture1 texture = pmdl::ReadTexture(file, &header, i);

		std::string path = directory + "/" + std::string(texture.path, texture.pathSize);
		Log::Info("Model Loader", "Loading texture: %s", path.c_str());
		textures[i].LoadFromFile(path);
	}

	for (uint16_t i = 0; i < header.materialCount; i++)
	{
		pmdl::Material1 mat = pmdl::ReadMaterial1(file, &header, i);

		Material* material = ResourceManager::GetInstance().NewMaterial();

		material->albedoColour = { mat.albedo.x, mat.albedo.y, mat.albedo.z, mat.albedo.w };
		material->albedo = ResourceManager::GetInstance().GetWhiteTexture();
		material->roughness = mat.roughness;
		material->metallic = mat.metallic;

		if (mat.albedoIndex != -1)
		{


			material->albedo = ResourceManager::GetInstance().NewTexture();
			ResourceManager().GetInstance().GetTexturePtr(material->albedo)->CreateFromImage(textures[mat.albedoIndex], true, true);
		}

		materials.push_back(material);
	}

	fclose(file);

	// inherited from Resource. This signals
	this->ready.store(true);
	
}

void Model::Queue(RenderManager* renderManager, glm::mat4 matrix, uint32_t lodIndex)
{
	// If it isn't ready to draw. Return and don't queue the meshes
	if (!ready.load())
		return;

	for (size_t i = 0; i < submeshes.size(); i++)
	{
		if (lodIndex == 0)
			renderManager->QueueMesh(mesh, materials[submeshes[i].materialIndex], matrix, submeshes[i].firstIndex, submeshes[i].indexCount, submeshes[i].firstVertex);
		else 
			renderManager->QueueMesh(mesh, materials[submeshes[i].materialIndex], matrix, LODs[submeshes[i].firstLOD + lodIndex - 1].firstIndex, LODs[submeshes[i].firstLOD + lodIndex - 1].indexCount, submeshes[i].firstVertex);
	}
}