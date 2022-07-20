#version 450 core

layout(location = 0) out vec4 outColour;

layout(location = 0) in vec2 vTexCoord;

layout(set = 0, binding = 0) uniform MaterialParams
{
	vec4 albedo;

} in_params;

layout(set = 0, binding = 1) uniform sampler2D in_texture;

void main()
{
	vec4 albedo = texture(in_texture, vTexCoord) * in_params.albedo;

	outColour = albedo;
}
