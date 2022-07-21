#version 450
layout(location = 0) out vec4 outColour;

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D in_texture;

void main()
{
	outColour = texture(in_texture, inUV);
}