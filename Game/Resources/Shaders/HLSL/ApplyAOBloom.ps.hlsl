


Texture2D currentColourBuffer : register(t0);
SamplerState colourBufferSampler : register(s0);

Texture2D bloom : register(t1);
SamplerState bloomSampler : register(s1);

Texture2D aoBuffer : register(t2);
SamplerState aoSampler : register(s2);


float4 main([[vk::location(0)]] float2 uv : TEXCOORD0) : SV_TARGET0
{

	float4 currentSample = currentColourBuffer.Sample(colourBufferSampler, uv);
	float3 bloomSample = bloom.Sample(bloomSampler, uv).rgb;

	float ao = aoBuffer.Sample(aoSampler, uv).r;

	//currentSample *= ao;

	currentSample += float4(bloomSample, 0.0);

	return currentSample;
}