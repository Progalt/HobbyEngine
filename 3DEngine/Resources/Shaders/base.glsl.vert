#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

layout(push_constant)  uniform Constants
{
	mat4 prevModel;
	mat4 model;
} constants;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec3 vNormal;

layout(location = 2) out vec4 vNewPos;
layout(location = 3) out vec4 vOldPos;

layout(set = 1, binding = 0) uniform GlobalData
{
	mat4 jitteredVP;
	mat4 VP;

	mat4 prevVP;
	vec4 viewPos;
	mat4 invProj;
	mat4 invView;
	mat4 view;
	mat4 proj;
} global;

void main()
{
	vNormal = mat3(transpose(inverse(constants.model))) * aNormal;
	vTexCoord = aTexCoord;
	gl_Position = global.jitteredVP * constants.model * vec4(aPosition, 1.0);

	vNewPos = global.VP * constants.model * vec4(aPosition, 1.0);
	vOldPos = global.prevVP * constants.prevModel * vec4(aPosition, 1.0);
}