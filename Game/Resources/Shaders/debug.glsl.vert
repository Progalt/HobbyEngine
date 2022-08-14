#version 450 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColour;

layout(push_constant) uniform PushConstants
{
	mat4 mvp;
} constants;

layout(location = 0) out vec4 vColour;

void main()
{
	gl_Position = constants.mvp * vec4(aPosition, 1.0);

	vColour = aColour;
}