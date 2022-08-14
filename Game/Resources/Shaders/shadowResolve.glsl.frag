
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
	int range = 1;
	
	for (int x = -range; x <= range; x++) {
		for (int y = -range; y <= range; y++) {
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y), cascadeIndex, bias);
			count++;
		}
	}
	return shadowFactor / count;
}

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);



void main()
{
	float sampledDepth = texture(gDepth, UV).r;
	vec3 fragPos = WorldPosFromDepth(sampledDepth, global.invProj, global.invView, UV, 1);

	vec4 viewPos = global.view *  vec4(fragPos, 1.0);

	vec4 shadowCoord;
	uint cascadeIndex = 0;

	// Get cascade index
	//uint cascadeIndex = 0;
	for(uint i = 0; i < CASCADE_COUNT - 1; ++i) {
		if(viewPos.z < shadowData.splitDepths[i]) {	
			cascadeIndex = i + 1;
		}
	}

	vec3 normal = decode(texture(gNormal, UV).rg);
	float NoL = dot(normal, vec3(sceneData.dirLight.direction));

	//cascadeIndex = 0;
	mat4 m = shadowData.matrices[cascadeIndex];

	shadowCoord = (biasMat * m) * vec4(fragPos, 1.0);	

	float biasMax = 0.03;
	float biasMin = 0.005;
	float bias = max(biasMax * 1.0 - NoL, biasMin);
	//float bias = 0.003 / (cascadeIndex + 1) + 0.0005 * cascadeIndex;  

	//bias /= cascadeIndex + 1;

	float shadow = 1;
	//if (constants.useIBL == 1)
		//shadow = textureProj(shadowCoord / shadowCoord.w, vec2(0.0), cascadeIndex, bias);
		shadow = filterPCF(shadowCoord / shadowCoord.w, cascadeIndex, bias);


	resolvedShadow = vec4(shadow, 0, 0, 1);
}