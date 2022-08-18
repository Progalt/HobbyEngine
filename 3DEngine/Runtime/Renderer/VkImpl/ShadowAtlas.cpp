
#include "ShadowAtlas.h"
#include <glm/gtc/matrix_transform.hpp>

#include "../../Core/Log.h"
#include <thread>
#include <chrono>

void CascadeShadowMap::Create(vk::Device* device, QualitySetting quality)
{

	switch (quality)
	{
	case QualitySetting::UltraLow:
		size = CascadeSize_UltraLow;
		break;
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

	currentQuality = quality;
}

void CascadeShadowMap::Recreate(vk::Device* device, QualitySetting quality)
{
	atlas.Destroy();
	renderpass.Destroy();


	switch (quality)
	{
	case QualitySetting::UltraLow:
		size = CascadeSize_UltraLow;
		break;
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

	atlas.CreateRenderTarget(vk::FORMAT_D32_SFLOAT, size * CascadeCount, size, false, vk::ImageLayout::Undefined);

	vk::RenderpassCreateInfo createInfo{};
	createInfo.type = vk::RenderpassType::Offscreen;
	createInfo.depthAttachment = { &atlas };
	createInfo.loadDepth = false;
	createInfo.extentWidth = size * CascadeCount;
	createInfo.extentHeight = size;
	createInfo.depthClear = 1.0f;

	renderpass = device->NewRenderpass(&createInfo);

	currentQuality = quality;

	Log::Info("Renderer", "Recreated Cascade Shadow Atlas");
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


	const float cascadeSplitLambda = 0.92f;

	float cascadeSplits[CascadeCount + 1];
	

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

		glm::vec3 frustumCorners[8] = {
			glm::vec3(-1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f),
		};

		// Project frustum corners into world space
		glm::mat4 invCam = glm::inverse(proj * view);
		for (uint32_t i = 0; i < 8; i++) {
			glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
			frustumCorners[i] = invCorner / invCorner.w;
		}

		for (uint32_t i = 0; i < 4; i++) {
			glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
			frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
			frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
		}

		// Get frustum center
		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (uint32_t i = 0; i < 8; i++) {
			frustumCenter += frustumCorners[i];
		}
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (uint32_t i = 0; i < 8; i++) {
			float distance = glm::length(frustumCorners[i] - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		glm::vec3 maxExtents = glm::vec3(radius);
		glm::vec3 minExtents = -maxExtents;

		

		glm::vec3 lightDir = glm::normalize(-dirLight.direction);
		glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -radius * 6.0f, radius * 6.0f);

		glm::vec4 texSize = glm::vec4((float)size / 3, size, 1.0f, 1.0f);

		if (stabiliseCascades)
		{
			glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
			glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			shadowOrigin = shadowMatrix * shadowOrigin;
			float storedW = shadowOrigin.w;
			shadowOrigin = shadowOrigin * texSize / 2.0f;

			glm::vec4 roundedOrigin = glm::floor(shadowOrigin);
			glm::vec4 roundOffset = roundedOrigin - shadowOrigin;


			roundOffset = roundOffset * 2.0f / texSize;
			roundOffset.z = 0.0f;
			roundOffset.w = 0.0f;

			glm::mat4 shadowProj = lightOrthoMatrix;
			shadowProj[3] += roundOffset;
			lightOrthoMatrix = shadowProj;

		}

		// Store split distance and matrix in cascade
		data.splitDepths[i] = (nearClip + splitDist * clipRange) * -1.0f;
		data.matrices[i] = lightOrthoMatrix * lightViewMatrix;

		lastSplitDist = cascadeSplits[i];
	}


	if (uniformBuffer.Valid())
		uniformBuffer.SetData(sizeof(DataBuffer), &data);
}