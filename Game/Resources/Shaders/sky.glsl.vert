
#version 450 core

layout(location = 0) out vec3 vPos;
layout(location = 1) out vec3 vSun;
layout(location = 2) out float vTime;

layout(set = 0, binding = 0) uniform GlobalData
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

layout(push_constant) uniform PushConstants
{
	float time;
} constants; 

  const vec2 data[6] = vec2[](
    vec2(-1.0,  1.0), vec2(-1.0, -1.0),
    vec2( 1.0,  1.0), 
	vec2( 1.0, -1.0), vec2( 1.0,  1.0), 
	vec2(-1.0, -1.0));

void main()
{
	gl_Position = vec4(data[gl_VertexIndex], 0.0, 1.0);
    vPos = transpose(mat3(global.view)) * (global.invProj * -gl_Position).xyz;

    vSun = vec3(0.0, sin(constants.time * 0.01), cos(constants.time * 0.01));
	vTime = constants.time;
}