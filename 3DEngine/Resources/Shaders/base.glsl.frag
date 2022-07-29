#version 450 core

#include "Defines.glsl"

layout(location = 0) out vec4 outColour;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outVelocity;

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec3 vNormal;

layout(location = 2) in vec4 vNewPos;
layout(location = 3) in vec4 vOldPos;

layout(set = 0, binding = 0) uniform MaterialParams
{
	vec4 albedo;

} in_params;

layout(set = 0, binding = 1) uniform sampler2D in_texture;


vec2 encode (vec3 v)
{
	vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx))  * sign_not_zero(p)) : p;
}

void main()
{
	outVelocity.rg = vec2(0.0, 0.0);

	outNormal.rg = encode(vNormal);
	outNormal.a = 1.0;

	vec4 albedo = texture(in_texture, vTexCoord) * in_params.albedo;

	outColour = albedo;

	vec2 newPos = (vNewPos.xy / vNewPos.w);
	vec2 prePos = (vOldPos.xy / vOldPos.w);

	outVelocity = vec4(newPos - prePos, 0, 1);
}
