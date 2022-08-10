
#version 450 core
layout(location = 0) in vec2 UV;

layout(binding = 0) uniform sampler2D depthTex;

void main()
{
	float depth = texture(depthTex, UV).r;

	gl_FragDepth = 1.0 - depth;
}