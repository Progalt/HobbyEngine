
#version 450 core

layout(location = 0) out vec4 oColour;

layout(location = 0) in vec4 vColour;

void main()
{
	oColour = vColour;
}