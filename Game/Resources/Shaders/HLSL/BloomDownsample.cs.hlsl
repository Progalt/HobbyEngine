
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
	// result image and inputTexture must be the same size

	float2 targetResolution;
	resultImage.GetDimensions(targetResolution.x, targetResolution.y);

	float2 resolution;
	currentColourBuffer.GetDimensions(resolution.x, resolution.y);

	if (globalInvocId.x > resolution.x || globalInvocId.y > resolution.y)
		return;

	float2 uv = (float2(globalInvocId.xy) + 0.5f) / float2(resolution);

	float2 texelSize = 1.0f / resolution;

	float x = texelSize.x;
	float y = texelSize.y;

	float3 a = Sample(uv.x - 2 * x, uv.y + 2 * y);
	float3 b = Sample(uv.x, uv.y + 2 * y);
	float3 c = Sample(uv.x - 2 * x, uv.y + 2 * y);

	float3 d = Sample(uv.x - 2 * x, uv.y);
	float3 e = Sample(uv.x, uv.y);
	float3 f = Sample(uv.x + 2 * x, uv.y);

	float3 g = Sample(uv.x - 2 * x, uv.y - 2 * y);
	float3 h = Sample(uv.x, uv.y - 2 * y);
	float3 i = Sample(uv.x + 2 * x, uv.y - 2 * y);

	float3 j = Sample(uv.x - x, uv.y + y);
	float3 k = Sample(uv.x + x, uv.y + y);
	float3 l = Sample(uv.x - x, uv.y - y);
	float3 m = Sample(uv.x + x, uv.y - y);


	float3 downsampled = e * 0.125;
	downsampled += (a + c + g + i) * 0.03125;
	downsampled += (b + d + f + h) * 0.0625;
	downsampled += (j + k + l + m) * 0.125;

	uint2 targetPixel = uint2((uv * targetResolution));

	resultImage[targetPixel] = float4(downsampled, 1.0);
}