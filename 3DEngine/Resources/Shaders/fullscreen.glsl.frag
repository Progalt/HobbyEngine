#version 450

#include "Defines.glsl"

layout(location = 0) out vec4 outColour;

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D in_texture;

layout(push_constant) uniform PushConstants
{
	int tonemappingMode;
} constants;

float desaturationFactor = 0.7;

void main()
{
	outColour = texture(in_texture, inUV);

	if (constants.tonemappingMode == 1)
		outColour.rgb = filmic(outColour.rgb);
	else if (constants.tonemappingMode == 2)
		outColour.rgb = unreal(outColour.rgb);
	else if (constants.tonemappingMode == 3)
		outColour.rgb = uncharted2(outColour.rgb);
	else if (constants.tonemappingMode == 4)
		outColour.rgb = aces(outColour.rgb);

	

}