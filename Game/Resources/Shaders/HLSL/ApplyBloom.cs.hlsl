
RWTexture2D<float4> resultImage : register(u0);

Texture2D currentColourBuffer : register(t1);
SamplerState colourBufferSampler : register(s1);

Texture2D bloomBuffer : register(t2);
SamplerState bloomSampler : register(s2);

static const float bloomStrength = 0.07f;



[numthreads(32, 32, 1)]
void main(uint3 globalInvocId : SV_DispatchThreadID)
{
	float2 resolution;
	currentColourBuffer.GetDimensions(resolution.x, resolution.y);

	float2 uv = (float2(globalInvocId.xy) + 0.5) / resolution;

	float4 colour = currentColourBuffer.SampleLevel(colourBufferSampler, uv, 0);
	float4 bloom = bloomBuffer.SampleLevel(bloomSampler, uv, 0);
	

	bloom *= 5.0f;

	resultImage[globalInvocId.xy] = lerp(colour, bloom, bloomStrength);
}