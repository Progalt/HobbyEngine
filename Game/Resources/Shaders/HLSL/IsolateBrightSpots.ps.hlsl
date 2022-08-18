

Texture2D currentColourBuffer : register(t0);
SamplerState colourBufferSampler : register(s0);

Texture2D emissiveBuffer : register(t1);
SamplerState emissiveSampler : register(s1);

struct VSInput
{
	[[vk::location(0)]] float2 uv : TEXCOORD0;
};

float4 main(VSInput input) : SV_TARGET0
{
	float4 sampledColour = currentColourBuffer.Sample(colourBufferSampler, input.uv);

	float4 sampledEmissive = emissiveBuffer.Sample(emissiveSampler, input.uv);

	float brightness = dot(sampledColour.rgb, float3(0.2126, 0.7152, 0.0722));

	if (brightness > 1.0)
		return sampledColour + sampledEmissive;
	else
		return sampledEmissive;

}