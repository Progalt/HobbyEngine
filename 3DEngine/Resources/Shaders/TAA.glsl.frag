#version 450 core

layout (location = 0) in vec2 outUV;

layout(location = 0) out vec4 outColour;

layout(binding = 0) uniform sampler2D in_hdr;
layout(binding = 1) uniform sampler2D in_velocity;
layout(binding = 2) uniform sampler2D in_history;

void main()
{
	float ModulationFactor = 1.0 / 16.0;

	vec2 Velocity = texture(in_velocity, outUV).rg;

	//vec2 Velocity = vec2(0.0);

	vec3 CurrentSubpixel = texture(in_hdr, outUV).rgb;
	vec3 History = texture(in_history, outUV + Velocity).rgb;

	outColour = vec4(mix(History, CurrentSubpixel, ModulationFactor), 1.0f);
}