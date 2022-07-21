#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

layout(push_constant)  uniform Constants
{
	mat4 mvp;
	mat4 model;
} constants;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec3 vNormal;

void main()
{
	vNormal = mat3(transpose(inverse(constants.model))) * aNormal;
	vTexCoord = aTexCoord;
	gl_Position = constants.mvp * vec4(aPosition, 1.0);
}