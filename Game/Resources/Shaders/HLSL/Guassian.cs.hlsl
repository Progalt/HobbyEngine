
Texture2D inputTexture : register(t0);
SamplerState inputSampler : register(s0);

RWTexture2D<float4> resultImage : register(u1);

struct PushConstants
{
	float blurScale;
	float blurStrength;
	int blurDirection;
};

[[vk::push_constant]] PushConstants constants;


static const float weight[5] =
{
	0.227027f,
	0.1945946f,
	0.1216216f,
	0.054054f,
	0.016216f
};

float3 blur(float2 uv, float2 resolution, float2 direction)
{
	float3 colour =0;
	float2 off1 = 1.3846153846 * direction;
	float2 off2 = 3.2307692308 * direction;

	colour += inputTexture.SampleLevel(inputSampler, uv, 0).rgb * 0.2270270270 * constants.blurStrength;
	colour += inputTexture.SampleLevel(inputSampler, uv + (off1 / resolution), 0).rgb * 0.3162162162 * constants.blurStrength;
	colour += inputTexture.SampleLevel(inputSampler, uv - (off1 / resolution), 0).rgb * 0.3162162162 * constants.blurStrength;
	colour += inputTexture.SampleLevel(inputSampler, uv + (off2 / resolution), 0).rgb * 0.0702702703 * constants.blurStrength;
	colour += inputTexture.SampleLevel(inputSampler, uv - (off2 / resolution), 0).rgb * 0.0702702703 * constants.blurStrength;

	return colour;
}

[numthreads(32, 32, 1)]
void main(uint3 globalInvocId : SV_DispatchThreadID)
{
	// result image and inputTexture must be the same size

	float2 resolution;
	inputTexture.GetDimensions(resolution.x, resolution.y);

	if (globalInvocId.x > resolution.x || globalInvocId.y > resolution.y)
		return;

	float2 uv = (float2(globalInvocId.xy) + 0.5f) / float2(resolution);

	float3 result = 0;

	/*float2 texOffset = 1.0f / resolution * constants.blurScale;
	float3 result = inputTexture.SampleLevel(inputSampler, uv, 0).rgb * weight[0];
	for (int i = 0; i < 5; i++)
	{
		if (constants.blurDirection == 1)
		{
			result += inputTexture.SampleLevel(inputSampler, uv + float2(texOffset.x * i, 0.0), 0).rgb * weight[i] * constants.blurStrength;
			result += inputTexture.SampleLevel(inputSampler, uv - float2(texOffset.x * i, 0.0), 0).rgb * weight[i] * constants.blurStrength;
		}
		else
		{
			result += inputTexture.SampleLevel(inputSampler, uv + float2(0.0, texOffset.y * i), 0).rgb * weight[i] * constants.blurStrength;
			result += inputTexture.SampleLevel(inputSampler, uv - float2(0.0, texOffset.y * i), 0).rgb * weight[i] * constants.blurStrength;
		}
	}*/

	if (constants.blurDirection == 1)
	{
		float2 dir1 = float2(0, 1) * constants.blurScale;
		float2 dir2 = float2(0, -1) * constants.blurScale;
		result += blur(uv, resolution, dir1);
		result += blur(uv, resolution, dir2);
	}
	else
	{
		float2 dir1 = float2(1, 0) * constants.blurScale;
		float2 dir2 = float2(-1, 0) * constants.blurScale;
		result += blur(uv, resolution, dir1);
		result += blur(uv, resolution, dir2);
	}
	

	resultImage[globalInvocId.xy] = float4(result, 1.0);
}
