
#include "Model.h"
#include "../Renderer/RenderManager.h"

#include "../Model/PMDL.h"

void Model::Discard()
{
	mesh->Destroy();
}

void Model::LoadFromFile(const std::string& path, RenderManager* renderManager)
{
	FILE* file = fopen(path.c_str(), "rb");

	pmdl::Header1 header = pmdl::ReadHeader1(file);

	pmdl::Vertex* vertices = pmdl::ReadVertices(file, &header);
	uint32_t* indices = pmdl::ReadIndices32bit(file, &header);

	this->mesh = renderManager->NewMesh();

	// Add all the vertex and index data to the mesh
	for (uint32_t i = 0; i < header.vertexCount; i++)
	{
		this->mesh->positions.push_back({ vertices[i].position.x, vertices[i].position.y, vertices[i].position.z });
		this->mesh->texCoords.push_back({ vertices[i].texCoord.x, vertices[i].texCoord.y });
		this->mesh->normals.push_back({ vertices[i].normal.x, vertices[i].normal.y, vertices[i].normal.z });
	}

	for (uint32_t i = 0; i < header.indexCount; i++)
		this->mesh->indices.push_back(indices[i]);

	PMDL_FREE(vertices);
	PMDL_FREE(indices);

	this->mesh->GenerateMesh();

	for (uint16_t i = 0; i < header.meshCount; i++)
	{
		pmdl::Mesh mesh = pmdl::ReadMesh1(file, &header, i);

		submeshes.push_back(mesh);
	}

	for (uint16_t i = 0; i < header.materialCount; i++)
	{
		pmdl::Material1 mat = pmdl::ReadMaterial1(file, &header, i);

		Material* material = ResourceManager::GetInstance().NewMaterial();

		material->albedoColour = { mat.albedo.x, mat.albedo.y, mat.albedo.z, mat.albedo.w };
		material->albedo = ResourceManager::GetInstance().GetWhiteTexture();

		materials.push_back(material);
	}



}

void Model::Queue(RenderManager* renderManager, glm::mat4 matrix)
{
	for (size_t i = 0; i < submeshes.size(); i++)
	{
		renderManager->QueueMesh(mesh, materials[submeshes[i].materialIndex], matrix, submeshes[i].firstIndex, submeshes[i].indexCount);
	}
}