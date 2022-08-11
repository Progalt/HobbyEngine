
#include "ShadowAtlas.h"
#include <glm/gtc/matrix_transform.hpp>

#include "../../Core/Log.h"

void CascadeShadowMap::Create(vk::Device* device, QualitySetting quality)
{
	if (currentQuality != QualitySetting::Undefined)
	{
		atlas.Destroy();
	}

	currentQuality = quality;

	switch (quality)
	{
	case QualitySetting::Low:
		size = CascadeSize_Low;
		break;
	case QualitySetting::Medium:
		size = CascadeSize_Medium;
		break;
	case QualitySetting::High:
		size = CascadeSize_High;
		break;
	}
	
	atlas = device->NewTexture();
	atlas.CreateRenderTarget(vk::FORMAT_D32_SFLOAT, size * CascadeCount, size, false, vk::ImageLayout::Undefined);


	vk::RenderpassCreateInfo createInfo{};
	createInfo.type = vk::RenderpassType::Offscreen;
	createInfo.depthAttachment = { &atlas };
	createInfo.loadDepth = false;
	createInfo.extentWidth = size * CascadeCount;
	createInfo.extentHeight = size;
	createInfo.depthClear = 1.0f;

	renderpass = device->NewRenderpass(&createInfo);

	uniformBuffer = device->NewBuffer();
	uniformBuffer.Create(vk::BufferType::Dynamic, vk::BufferUsage::Uniform, sizeof(data), nullptr);

	Log::Info("Renderer", "Created Cascade Shadow Atlas");
}

void CascadeShadowMap::CreateDescriptor(vk::DescriptorLayout* layout, vk::Device* device)
{
	descriptor = device->NewDescriptor(layout);

	descriptor.BindBuffer(&uniformBuffer, 0, sizeof(DataBuffer), 0);
	descriptor.Update();
	
	createdDescriptor = true;
}

void CascadeShadowMap::Destroy()
{
	atlas.Destroy();
	renderpass.Destroy();
	uniformBuffer.Destroy();
}

void CascadeShadowMap::SetupForRendering(vk::CommandList& cmdList, uint32_t cascadeIndex)
{
	
	if (!begunRenderpass)
	{
		cmdList.BeginRenderpass(&renderpass, false, size * CascadeCount, size);
		begunRenderpass = true;
	}

	cmdList.SetViewport(size * cascadeIndex, 0, size, size);
	cmdList.SetScissor(size * cascadeIndex, 0, size, size);
}

void CascadeShadowMap::FinishRendering(vk::CommandList& cmdList)
{
	begunRenderpass = false;
	cmdList.EndRenderpass();
}

void CascadeShadowMap::UpdateCascades(DirectionalLight& dirLight, float nearClip, float farClip, const glm::mat4& proj, const glm::mat4& view)
{

	const float cascadeSplitLambda = 0.925f;

	float cascadeSplits[CascadeCount];

	float clipRange = farClip - nearClip;

	float minZ = nearClip;
	float maxZ = nearClip + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;


	for (uint32_t i = 0; i < CascadeCount + 1; i++) {
		float p = (i + 1) / static_cast<float>(CascadeCount + 1);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearClip) / clipRange;
	}

	float lastSplitDist = 0.0;
	for (uint32_t i = 0; i < CascadeCount; i++)
	{
		float splitDist = cascadeSplits[i];

		glm::vec3 frustumCornersWS[8] =
		{
			glm::vec3(-1.0f, 1.0f, -1.0f),
			glm::vec3(1.0f, 1.0f, -1.0f),
			glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, -1.0f, 1.0f),
			glm::vec3(-1.0f, -1.0f, 1.0f),
		};

		glm::mat4 invViewProj = glm::inverse(proj * view);
		for (unsigned int i = 0; i < 8; ++i)
		{
			glm::vec4 inversePoint = invViewProj * glm::vec4(frustumCornersWS[i], 1.0f);
			frustumCornersWS[i] = glm::vec3(inversePoint / inversePoint.w);
		}

		for (unsigned int i = 0; i < 4; ++i)
		{
			glm::vec3 cornerRay = frustumCornersWS[i + 4] - frustumCornersWS[i];
			glm::vec3 nearCornerRay = cornerRay * lastSplitDist;
			glm::vec3 farCornerRay = cornerRay * splitDist;
			frustumCornersWS[i + 4] = frustumCornersWS[i] + farCornerRay;
			frustumCornersWS[i] = frustumCornersWS[i] + nearCornerRay;
		}

		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (unsigned int i = 0; i < 8; ++i)
			frustumCenter += frustumCornersWS[i];
		frustumCenter /= 8.0f;


		float radius = 0.0f;
		for (unsigned int i = 0; i < 8; ++i)
		{
			float distance = glm::length(frustumCornersWS[i] - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		glm::vec3 maxExtents = glm::vec3(radius, radius, radius);
		glm::vec3 minExtents = -maxExtents;

		glm::vec3 lightDirection = frustumCenter - glm::normalize(glm::vec3(-dirLight.direction)) * -minExtents.z;
		glm::mat4 lightViewMatrix = glm::mat4(1.0f);
		lightViewMatrix = glm::lookAt(lightDirection, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec3 cascadeExtents = maxExtents - minExtents;

		glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -cascadeExtents.z * 4, cascadeExtents.z);

		// Store split distance and matrix in cascade
		data.splitDepths[i] = (nearClip + splitDist * clipRange) * -1.0f;
		data.matrices[i] = lightOrthoMatrix * lightViewMatrix;

		lastSplitDist = cascadeSplits[i];
	}


	if (uniformBuffer.Valid())
		uniformBuffer.SetData(sizeof(DataBuffer), &data);
}