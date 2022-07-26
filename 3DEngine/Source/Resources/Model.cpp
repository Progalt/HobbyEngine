
#include "Model.h"
#include "../Renderer/RenderManager.h"

#include "../Model/PMDL.h"

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

	this->mesh = renderManager->NewMesh();

	this->mesh->positions.resize(header.vertexCount);
	this->mesh->texCoords.resize(header.vertexCount);
	this->mesh->normals.resize(header.vertexCount);

	// Add all the vertex and index data to the mesh
	for (uint32_t i = 0; i < header.vertexCount; i++)
	{
		// PMDL outputs a vertex struct currently
		// so convert to our system
		this->mesh->positions[i] = { vertices[i].position.x, vertices[i].position.y, vertices[i].position.z };
		this->mesh->texCoords[i] = { vertices[i].texCoord.x, vertices[i].texCoord.y };
		this->mesh->normals[i] = { vertices[i].normal.x, vertices[i].normal.y, vertices[i].normal.z };
	}

	this->mesh->indices.resize(header.indexCount);

	for (uint32_t i = 0; i < header.indexCount; i++)
		this->mesh->indices[i] = indices[i];

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

	fclose(file);

	// inherited from Resource. This signals
	this->ready.store(true);

}

void Model::Queue(RenderManager* renderManager, glm::mat4 matrix)
{
	// If it isn't ready to draw. Return and don't queue the meshes
	if (!ready.load())
		return;

	for (size_t i = 0; i < submeshes.size(); i++)
	{
		renderManager->QueueMesh(mesh, materials[submeshes[i].materialIndex], matrix, submeshes[i].firstIndex, submeshes[i].indexCount);
	}
}