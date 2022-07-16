
#include "MeshVk.h"


MeshVk::MeshVk(vk::Device* dev)
{
	this->device = dev;
}


void MeshVk::GenerateMesh()
{
	// calculate the bounding box
	// this calculates the base bounding box with no transformation
	boundingBox.CalculateFromVertexData(positions);

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 texCoord;
		glm::vec3 normal;
	};

	// Pack into a vertex structure
	// Purely because I need to implement de interleaved later
	std::vector<Vertex> vertices(positions.size());

	for (size_t i = 0; i < positions.size(); i++)
	{
		vertices[i].pos = positions[i];
		//vertices[i].texCoord = texCoords[i];
		//vertices[i].normal = normals[i];
	}

	for (size_t i = 0; i < texCoords.size(); i++)
		vertices[i].texCoord = texCoords[i];

	for (size_t i = 0; i < normals.size(); i++)
		vertices[i].normal = normals[i];

	//Create the vertex buffer 

	if (!vertexBuffer.Valid())
		vertexBuffer = device->NewBuffer();

	if (vertexBuffer.GetSize() == 0 || vertexBuffer.GetSize() != vertices.size() * sizeof(Vertex))
	{
		if (vertexBuffer.GetSize() != 0)
			vertexBuffer.Destroy();

		vertexBuffer.Create(vk::BufferType::Static, vk::BufferUsage::Vertex, vertices.size() * sizeof(Vertex), vertices.data());
	}

	if (!indexBuffer.Valid())
		indexBuffer = device->NewBuffer();

	if (indexBuffer.GetSize() == 0 || indexBuffer.GetSize() != indices.size() * sizeof(IndexType))
	{
		if (indexBuffer.GetSize() != 0)
			indexBuffer.Destroy();

		indexBuffer.Create(vk::BufferType::Static, vk::BufferUsage::Index, indices.size() * sizeof(IndexType), indices.data());
	}
}