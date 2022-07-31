
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
		this->mesh->positions[i] = { vertices[i].x, vertices[i].y, vertices[i].z };
		this->mesh->texCoords[i] = { vertices[i].u, vertices[i].v };
		this->mesh->normals[i] = { vertices[i].nx, vertices[i].ny, vertices[i].nz };
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

		for (uint16_t l = mesh.firstLOD; l < mesh.firstLOD + mesh.LODCount; l++)
			LODs.push_back(pmdl::ReadLOD1(file, &header, l));

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