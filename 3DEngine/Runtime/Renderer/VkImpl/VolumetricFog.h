#pragma once

#include "../Vulkan/Device.h"
#include "ShadowAtlas.h"

// Volumetric fog is something I've wanted to implement for a while 
// How it works: 
// Based on a few presentations done previously
// use a low res 3d texture as a voxel grid


constexpr glm::uvec3 GridRes_Low = { 80, 45, 64 };
constexpr glm::uvec3 GridRes_High = { 160, 90, 128 };

class VolumetricFogHandler
{
public:

	//void Create(QualitySetting quality);


	vk::Texture voxelGrid;

private:
};