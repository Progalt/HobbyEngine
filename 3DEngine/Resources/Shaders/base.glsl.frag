#version 450 core

#include "Defines.glsl"

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outVelocity;
layout(location = 3) out vec4 outEmissive;

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec3 vNormal;

layout(location = 2) in vec4 vNewPos;
layout(location = 3) in vec4 vOldPos;

layout(location = 4) in vec3 vTangent;

layout(set = 0, binding = 0) uniform MaterialParams
{
	vec4 albedo;

	float roughness;
	float metallic;

	int hasNormal;
	int roughnessMetallicPacked;

	vec4 emissive;

} in_params;

layout(set = 0, binding = 1) uniform sampler2D in_texture;

layout(set = 0, binding = 2) uniform sampler2D in_normal;

layout(set = 0, binding = 3) uniform sampler2D in_roughness;

layout(set = 0, binding = 4) uniform sampler2D in_metallic;

vec3 calculateNormal()
{
	if (in_params.hasNormal == 0)
		return normalize(vNormal);

	vec3 tangentNormal = texture(in_normal, vTexCoord).xyz * 2.0 - 1.0;

	vec3 N = normalize(vNormal);
	vec3 T = normalize(vTangent.xyz);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	return normalize(TBN * tangentNormal);
}


void main()
{
	outVelocity.rg = vec2(0.0, 0.0);



	outColour.a = 1.0;

	outNormal.rg = encode(calculateNormal());
	if (in_params.roughnessMetallicPacked == 0)
	{
		outNormal.b = texture(in_roughness, vTexCoord).r * in_params.roughness;
		outNormal.a = texture(in_metallic, vTexCoord).r * in_params.metallic;
	}
	else
	{
		outNormal.b = texture(in_roughness, vTexCoord).g * in_params.roughness;
		outNormal.a = texture(in_roughness, vTexCoord).b * in_params.metallic;
	}

	vec4 albedo = texture(in_texture, vTexCoord) * in_params.albedo;

	outColour.rgb = albedo.rgb;

	outEmissive.rgba = vec4(0.0, 0.0, 0.0, 1.0);
	outEmissive.rgb = in_params.emissive.rgb;

	vec2 newPos = (vNewPos.xy / vNewPos.w);
	vec2 prePos = (vOldPos.xy / vOldPos.w);

	outVelocity = vec4(newPos - prePos, 0, 1);
}
