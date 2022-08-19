
#include "Model.h"
#include "../Renderer/RenderManager.h"
#include "../Renderer/Mesh.h"
#include "../Model/PMDL.h"
#include "../Core/Log.h"

void Model::Discard()
{
	mesh->Destroy();
	delete mesh;
}

void Model::LoadFromFile(const std::string& path, RenderManager* renderManager, bool loadTextures)
{
	// Currently PMDL doesn't have a sort of global loadModel function
	// Each section must be loaded as needed

	// Its quite fast to load and convert to our format because most it is fairly simple

	FILE* file = fopen(path.c_str(), "rb");

	pmdl::Header1 header = pmdl::ReadHeader1(file);
	
	Log::Info("Model Loader", "Loading Model with %d vertices and %d indices", header.vertexCount, header.indexCount);

	pmdl::Vertex* vertices = pmdl::ReadVertices(file, &header);
	uint32_t* indices = pmdl::ReadIndices32bit(file, &header);

	Log::Info("Model Loader", "Loaded Vertices and indices");

	mesh = renderManager->NewMesh();

	mesh->positions.resize(header.vertexCount);
	mesh->texCoords.resize(header.vertexCount);
	mesh->normals.resize(header.vertexCount);
	mesh->tangents.resize(header.vertexCount);

	// Add all the vertex and index data to the mesh
	for (uint32_t i = 0; i < header.vertexCount; i++)
	{
		// PMDL outputs a vertex struct currently
		// so convert to our system
		mesh->positions[i] = { vertices[i].x, vertices[i].y, vertices[i].z };
		mesh->texCoords[i] = { vertices[i].u, vertices[i].v };
		mesh->normals[i] = { vertices[i].nx, vertices[i].ny, vertices[i].nz };
		mesh->tangents[i] = { vertices[i].tx, vertices[i].ty, vertices[i].tz };
	}

	mesh->indices.resize(header.indexCount);

	for (uint32_t i = 0; i < header.indexCount; i++)
		this->mesh->indices[i] = indices[i];

	PMDL_FREE(vertices);
	PMDL_FREE(indices);

	this->mesh->GenerateMesh();

	Log::Info("Model Loader", "Generated Mesh");
	Log::Info("Model Loader", "Mesh Count: %d", header.meshCount);

	for (uint16_t i = 0; i < header.meshCount; i++)
	{
		pmdl::Mesh mesh = pmdl::ReadMesh1(file, &header, i);

		Mesh::DrawCall drawCall;
		drawCall.firstIndex = mesh.firstIndex;
		drawCall.firstVertex = mesh.firstVertex;
		drawCall.indexCount = mesh.indexCount;
		Log::Info("Model Loader", "BB Min: %.4f, %.4f, %.4f", mesh.boundingBox.min.x, mesh.boundingBox.min.y, mesh.boundingBox.min.z);
		Log::Info("Model Loader", "BB Max: %.4f, %.4f, %.4f", mesh.boundingBox.max.x, mesh.boundingBox.max.y, mesh.boundingBox.max.z);
		drawCall.boundingBox.untransformed_min = { mesh.boundingBox.min.x, mesh.boundingBox.min.y, mesh.boundingBox.min.z };
		drawCall.boundingBox.untransformed_max = { mesh.boundingBox.max.x, mesh.boundingBox.max.y, mesh.boundingBox.max.z };

		//this->mesh->AddDrawCall(drawCall);

		this->mesh->drawCalls.push_back(drawCall);

		submeshes.push_back(mesh);
	}

	Log::Info("Model Loader", "Read Mesh Data from file");

	struct Img
	{
		std::string name;
		Image img;
	};

	uint32_t textureCount = (loadTextures) ? header.textureCount : 0;

	std::vector<Img> textures(textureCount);

	std::string directory = FileSystem::GetDirectory(path);


	for (uint16_t i = 0; i < textureCount; i++)
	{
		pmdl::Texture1 texture = pmdl::ReadTexture(file, &header, i);

		std::string path = directory + "/" + std::string(texture.path, texture.pathSize);
		Log::Info("Model Loader", "Loading texture: %s", path.c_str());
		textures[i].img.LoadFromFile(path);
		textures[i].name = std::string(texture.path, texture.pathSize);
	}

	Log::Info("Model Loader", "Loaded Textures");

	for (uint16_t i = 0; i < header.materialCount; i++)
	{
		pmdl::Material1 mat = pmdl::ReadMaterial1(file, &header, i);

		Material* material = ResourceManager::GetInstance().NewMaterial();

		material->albedoColour = { mat.albedo.x, mat.albedo.y, mat.albedo.z, mat.albedo.w };
		material->albedo = ResourceManager::GetInstance().GetWhiteTexture();
		material->roughnessMap = ResourceManager::GetInstance().GetWhiteTexture();
		//material->normalMap = ResourceManager::GetInstance().GetWhiteTexture();
		material->metallicMap = ResourceManager::GetInstance().GetWhiteTexture();
		material->roughness = mat.roughness;
		material->metallic = mat.metallic;

		if (loadTextures)
		{
			if (mat.albedoIndex != -1)
			{
				material->albedo = ResourceManager::GetInstance().NewTexture(textures[mat.albedoIndex].name);
				if (!ResourceManager().GetInstance().GetTexturePtr(material->albedo)->created)
					ResourceManager().GetInstance().GetTexturePtr(material->albedo)->CreateFromImage(textures[mat.albedoIndex].img, true, true);
			}

			if (mat.normalIndex != -1)
			{
				material->normalMap = ResourceManager::GetInstance().NewTexture(textures[mat.normalIndex].name);
				if (!ResourceManager().GetInstance().GetTexturePtr(material->normalMap)->created)
					ResourceManager().GetInstance().GetTexturePtr(material->normalMap)->CreateFromImage(textures[mat.normalIndex].img, true, false);
			}

			if (mat.roughnessIndex != -1)
			{
				material->roughnessMap = ResourceManager::GetInstance().NewTexture(textures[mat.roughnessIndex].name);
				if (!ResourceManager().GetInstance().GetTexturePtr(material->roughnessMap)->created)
					ResourceManager().GetInstance().GetTexturePtr(material->roughnessMap)->CreateFromImage(textures[mat.roughnessIndex].img, true, true);
			}

			if (mat.metallicIndex != -1)
			{
				material->metallicMap = ResourceManager::GetInstance().NewTexture(textures[mat.metallicIndex].name);
				if (!ResourceManager().GetInstance().GetTexturePtr(material->metallicMap)->created)
					ResourceManager().GetInstance().GetTexturePtr(material->metallicMap)->CreateFromImage(textures[mat.metallicIndex].img, true, true);
			}

			if (material->metallicMap == material->roughnessMap)
				material->roughnessMetallicShareTexture = true;
		}

		materials.push_back(material);
	}

	Log::Info("Model Loader", "Loaded Materials: %d", header.materialCount);


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
			renderManager->QueueMesh(mesh, materials[submeshes[i].materialIndex], matrix, submeshes[i].firstIndex, submeshes[i].indexCount, submeshes[i].firstVertex, i);
		else 
			renderManager->QueueMesh(mesh, materials[submeshes[i].materialIndex], matrix, LODs[submeshes[i].firstLOD + lodIndex - 1].firstIndex, LODs[submeshes[i].firstLOD + lodIndex - 1].indexCount, submeshes[i].firstVertex, i);
	}
}