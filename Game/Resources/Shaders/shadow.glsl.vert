
#version 450 core

#include "Defines.glsl"

// vertices are currently interleaved
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

layout(push_constant) uniform PushConstants
{
	mat4 model;
	int cascadeIndex;
} constants;

layout(binding = 0) uniform ShadowData
{
	mat4 matrices[CASCADE_COUNT];
	vec4 splitDepths;
} data;

void main()
{
	gl_Position = data.matrices[constants.cascadeIndex] * constants.model * vec4(aPosition, 1.0); 
}