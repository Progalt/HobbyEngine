#pragma once

#include <vector>
#include <string>

enum class PostProcessInput
{
	// This is the current colour target outputted from the lighting and previous post process
	Colour,

	// Depth of the scene
	Depth, 

	// Normal, roughness and metallic are passed in one texture 
	// Normal must be decoded to be used
	// Or the normal could be reconstructed in the shader
	Normal_Roughness_Metallic,

	// Velocity of the scene
	Velocity,

	Emissive
};

struct PostProcessCreateInfo
{
	// Texture inputs
	std::vector<PostProcessInput> inputs;

	// Size of uniform buffer to bind to it
	uint32_t uniformBufferSize;

	// Passes global data struct with projection information into descriptor set 0
	bool passGlobalData;

	// Is the post process effect in a compute shader
	// Shouldn't affect the set up much but does under the hood
	// Some further limitations:
	// For now - the local_size must be X:16 and Y:16
	bool computeShader;

	// shader byte code
	std::vector<int8_t> shaderByteCode;

	// This must only be for one post process effect. 
	// This is where it will cache the history after
	// Useful for after the TAA pass for example.
	bool cacheHistory = false;
};

class PostProcessEffect
{
public:

	// The size of the data submitted must match uniformBufferSize specified in the createInfo
	virtual void UpdateUniformBuffer(void* data) = 0;

	virtual void Destroy() = 0;

	std::string name;

	bool enabled = true;


	PostProcessCreateInfo createInfo;

private:
};