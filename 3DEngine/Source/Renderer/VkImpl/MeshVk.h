#pragma once


#include "../Mesh.h"

#include "../Vulkan/Device.h"

class MeshVk : public Mesh
{
public:

	MeshVk(vk::Device* dev);


	void GenerateMesh() override;


	vk::Device* device;

	// Buffers

	vk::Buffer vertexBuffer;
	vk::Buffer indexBuffer;

private:
};