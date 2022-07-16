#pragma once

#include <string>
#include "../FileSystem/ResourceManager.h"
#include <unordered_map>
#include <glm/glm.hpp>

enum class TextureType
{
	// All the basic PBR textures
	Albedo,
	Normal,
	Roughness,
	Metallic,
	AmbientOcclusion,

	// These textures are not used by the pbr pipeline but custom shaders might use them
	// For instance additional 1 could be used as displacement
	Additional1,
	Additional2,
	Additional3
};

class Material
{
public:

	std::string name;

	std::string shaderName;

	// Does the object with this material cast shadows
	// If this is false. It is excluded from shadow rendering passes
	bool castsShadows;

	std::unordered_map<TextureType, Handle<Texture>> textures;

	// These values control the material
	// If a texture is supplied it will be a modifier for the texture of that type
	// these values are between 0 and 1
	glm::vec4 colour;
	float roughness;
	float metallic;
	
	// This only works if an AO texture map is supplied
	// between 0 and 1
	float aoStrength;

private:
};