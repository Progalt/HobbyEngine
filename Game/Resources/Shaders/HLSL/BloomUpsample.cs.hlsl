
Texture2D currentColourBuffer : register(t0);
SamplerState colourBufferSampler : register(s0);

RWTexture2D<float4> resultImage : register(u1);

float3 Sample(float x, float y)
{
	return currentColourBuffer.SampleLevel(colourBufferSampler, float2(x, y), 0).rgb;
}

[numthreads(32, 32, 1)]
void main(uint3 globalInvocId : SV_DispatchThreadID)
{
	// On upscaling the target resolution is used to determine the numthreads
	float2 targetResolution;
	resultImage.GetDimensions(targetResolution.x, targetResolution.y);

	float2 resolution;
	currentColourBuffer.GetDimensions(resolution.x, resolution.y);

	if (globalInvocId.x > targetResolution.x || globalInvocId.y > targetResolution.y)
		return;

	float2 filterRadius = 1.0 / resolution;

	float2 uv = (float2(globalInvocId.xy) + 0.5) / targetResolution;

	float x = filterRadius.x;
	float y = filterRadius.y;

	float3 a = Sample(uv.x - x, uv.y + y).rgb;
	float3 b = Sample(uv.x,     uv.y + y).rgb;
	float3 c = Sample(uv.x + x, uv.y + y).rgb;

	float3 d = Sample(uv.x - x, uv.y).rgb;
	float3 e = Sample(uv.x,     uv.y).rgb;
	float3 f = Sample(uv.x + x, uv.y).rgb;

	float3 g = Sample(uv.x - x, uv.y - y).rgb;
	float3 h = Sample(uv.x,     uv.y - y).rgb;
	float3 i = Sample(uv.x + x, uv.y - y).rgb;

	float3 upsample = e * 4.0;
	upsample += (b + d + f + h) * 2.0;
	upsample += (a + c + g + i);
	upsample *= 1.0 / 16.0;


	resultImage[globalInvocId.xy] = float4(upsample, 1.0);
}