#version 450 core

#define FXAA_REDUCE_MIN (1.0 / 128.0)
#define FXAA_REDUCE_MUL (1.0 / 8.0)
#define FXAA_SPAN_MAX 8.0

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColour;

layout(binding = 0) uniform sampler2D input_texture;


void main() {
	// Samples the texels around and calculate their corresponding luminosity
	vec2 texSize = textureSize(input_texture, 0);
	vec2 texel_size = vec2(1.0 / texSize.x, 1.0 / texSize.y);

	vec3 calculateLuma = vec3(0.299, 0.587, 0.114);
	vec3 rgbM  = texture(input_texture, uv).xyz;
	vec3 rgbNW = texture(input_texture, uv + (vec2(-1.0,-1.0)) * texel_size).xyz;
	vec3 rgbNE = texture(input_texture, uv + (vec2(1.0,-1.0)) * texel_size).xyz;
	vec3 rgbSW = texture(input_texture, uv + (vec2(-1.0,1.0)) * texel_size).xyz;
	vec3 rgbSE = texture(input_texture, uv + (vec2(1.0,1.0)) * texel_size).xyz;

	float lumaM  = dot(rgbM,  calculateLuma);
	float lumaNW = dot(rgbNW, calculateLuma);
	float lumaNE = dot(rgbNE, calculateLuma);
	float lumaSW = dot(rgbSW, calculateLuma);
	float lumaSE = dot(rgbSE, calculateLuma);
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))); 

	// Calculate sample direction
	vec2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * texel_size;

	// Perform the samples and calculate the new texel colour
	vec3 rgbA = 0.5 * (texture(input_texture, uv + dir * ((1.0 / 3.0) - 0.5)).xyz + texture(input_texture, uv + dir * ((2.0 / 3.0) - 0.5)).xyz);
	vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(input_texture, uv + dir * - 0.5).xyz + texture(input_texture, uv + dir * 0.5).xyz);
	float lumaB = dot(rgbB, calculateLuma);
	if ((lumaB < lumaMin) || (lumaB > lumaMax))
    {
		fragColour = vec4(rgbA, 1.0);
	} else 
    {
		fragColour = vec4(rgbB, 1.0);
	}
}