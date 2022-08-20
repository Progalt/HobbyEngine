#pragma once

#include "../FileSystem/ResourceManager.h"
#include <glm/glm.hpp>

enum class Pass
{
	DontCare, 
	Deferred, 
	Forward
};

// This is (going) to be a large material class
// I could do a fancy system where I read inputs into the shader and generate a texture using that
// but if I force one material layout it makes life easier
// Shaders don't have to use all inputs 

class Material
{
public:

	virtual void Discard() = 0;

	// Use this to override what pass the material is drawn in. 
	// Don't care means the renderer will decide what pass to put it in.
	Pass pass = Pass::DontCare;

	bool castShadows = true;

	int roughnessMetallicShareTexture = false;

	Handle<Texture> albedo;
	Handle<Texture> normalMap;
	Handle<Texture> roughnessMap;
	Handle<Texture> metallicMap;
	glm::vec4 albedoColour;

	glm::vec4 emissiveColour = { 0.0f, 0.0f, 0.0f, 1.0f };

	float roughness = 0.5f;
	float metallic = 0.5f;

	float translucency = 0.0f;

	// This updates material parameters.
	// Currently it does not update material textures
	bool setUpdate = false;

private:
};