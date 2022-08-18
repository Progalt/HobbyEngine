
#version 450 

#include "Defines.glsl"
#include "StructInclude.glsl"

layout(location = 0) out vec4 resolvedShadow;

layout(location = 0) in vec2 UV;

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



layout(set = 1, binding = 0) uniform sampler2DShadow cascadeAtlas;

layout(set = 1, binding = 1) readonly buffer SceneData
{
	uint hasDirectionalLight; 
	uint lightCount;

	float padding[2];

	DirectionalLight dirLight;
	PointLight[] pointLight;
} sceneData;

layout(set = 1, binding = 2) uniform sampler2D gDepth;

layout(set = 1, binding = 3) uniform ShadowData
{
	mat4 matrices[CASCADE_COUNT];
	vec4 splitDepths;
} shadowData;

layout(set = 1, binding = 4) uniform sampler2D gNormal;

float GetBias(float NdotL, float biasMul, float baseBias)
{
	float factor = 0.01;

	float saturated_ndotl =  clamp(NdotL, 0, 1);

	float slopeFactor = 1.0 - saturated_ndotl;

	float finalBias = factor * slopeFactor * baseBias * biasMul;

	return finalBias;
}

vec3 NormalBiasOffset(vec3 n, float NdotL, float normalBias)
{
	vec3 texelSize = vec3(1.0 / textureSize(cascadeAtlas, 0), 0.0);

	return n * (1.0 - clamp(NdotL, 0, 1)) * normalBias * texelSize * 10.0;
}


float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex, float bias)
{
	float shadow = 1.0;

	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) {

		vec2 sampleCoord = shadowCoord.st + offset;

		float cascadeSize = 1.0 / 3.0;
		
		sampleCoord.x /= 3;

		sampleCoord.x += cascadeSize * cascadeIndex;

		/*float dist = texture(cascadeAtlas, sampleCoord).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
			shadow = 0.0;
		}*/

		shadow = texture(cascadeAtlas, vec3(sampleCoord, shadowCoord.z - bias));

		//shadow = sampleVariance(sampleCoord, shadowCoord.z - bias);
	}
	return shadow;

}

float filterPCF(vec4 sc, uint cascadeIndex, float bias)
{

	ivec2 texDim = textureSize(cascadeAtlas, 0).xy;
	texDim.x /= 3;
	float scale = 1.2;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 2;
	
	for (int x = -range; x <= range; x++) {
		for (int y = -range; y <= range; y++) {
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y), cascadeIndex, bias);
			count++;
		}
	}
	return shadowFactor / count;
}


const float  blendThreshold = 0.7;

void main()
{
	float sampledDepth = texture(gDepth, UV).r;

	vec3 normal = decode(texture(gNormal, UV).rg);
	float NoL = dot(normal, vec3(-sceneData.dirLight.direction));

	vec3 fragPos = WorldPosFromDepth(sampledDepth, global.invProj, global.invView, UV, 1) + NormalBiasOffset(normal, NoL, 20.0);

	// Transform the world position into a view position
	vec4 viewPos = global.view *  vec4(fragPos, 1.0);

	vec4 shadowCoord;
	uint cascadeIndex = 0;

	// Get cascade index
	for(uint i = 0; i < CASCADE_COUNT - 1; i++) {
		if(viewPos.z < shadowData.splitDepths[i]) {	
			cascadeIndex = i + 1;
		}
	}

	vec3 posNdc = WorldToNDC(fragPos,  shadowData.matrices[cascadeIndex]);
	vec2 posUv = NDCToUV(posNdc.xy);

	//cascadeIndex = 0;
	mat4 m = shadowData.matrices[cascadeIndex];


	shadowCoord = vec4(posUv, posNdc.z, 1.0);

	float biasbase = 0.07;
	float mul = 1.0 / float(cascadeIndex + 1);
	float bias = GetBias(NoL, mul, biasbase);
	
	float shadow = 1;
	
	shadow = textureProj(shadowCoord, vec2(0.0), cascadeIndex, bias);
	shadow = filterPCF(shadowCoord / shadowCoord.w, cascadeIndex, bias);

	float fade = clamp((1.0 - viewPos.z / shadowData.splitDepths[cascadeIndex]) / 0.5, 0, 1);
	fade = 1.0 - fade;

	if (fade < 1.0 && cascadeIndex < CASCADE_COUNT - 1)
	{
		cascadeIndex++;

		vec3 posNdc = WorldToNDC(fragPos,  shadowData.matrices[cascadeIndex]);
		vec2 posUv = NDCToUV(posNdc.xy);

		//cascadeIndex = 0;
		mat4 m = shadowData.matrices[cascadeIndex];


		shadowCoord = vec4(posUv, posNdc.z, 1.0);

		float mul = 1.0 / float(cascadeIndex + 1);
		float bias = GetBias(NoL, mul, biasbase);
	
		float secondShadow = 1;
	
		secondShadow = textureProj(shadowCoord, vec2(0.0), cascadeIndex, bias);
		secondShadow = filterPCF(shadowCoord / shadowCoord.w, cascadeIndex, bias);

		shadow = mix(shadow, secondShadow, fade);

	}

	resolvedShadow = vec4(shadow, 0, 0, 1);
}