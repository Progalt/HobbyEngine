

#version 450 


layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 fragColour;

layout(binding = 0) uniform sampler2D in_mainImage;
layout(binding = 1) uniform sampler2D in_AO;


void main()
{
	vec4 sampled = texture(in_mainImage, UV);
	float ao = texture(in_AO, UV).r;

	fragColour = sampled;
	fragColour.rgb *= ao;
}