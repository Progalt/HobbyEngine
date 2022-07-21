#version 450 core

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outVelocity;

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec3 vNormal;

layout(set = 0, binding = 0) uniform MaterialParams
{
	vec4 albedo;

} in_params;

layout(set = 0, binding = 1) uniform sampler2D in_texture;

vec2 encode (vec3 n)
{
   return n.xy * 0.5 + 0.5;
}

void main()
{
	outVelocity.rg = vec2(0.0, 0.0);

	outNormal.rg = normalize(encode(vNormal));
	outNormal.a = 1.0;

	vec4 albedo = texture(in_texture, vTexCoord) * in_params.albedo;

	outColour = albedo;
}
