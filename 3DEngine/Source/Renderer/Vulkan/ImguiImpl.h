#pragma once


/*#include "../Vendor/imgui/imgui.h"
#include "../Vendor/imgui/imgui_internal.h"
#include "../Vendor/imgui/imgui_impl_sdl.h"
#include "../Vendor/imgui/imgui_impl_vulkan.h"

class Window;

namespace vk
{

	class Device;
	class Renderpass;
	class CommandList;
	enum class ImageLayout;
	class Sampler;
	class Texture;

	class Imgui
	{
	public:

		static void Init(vk::Device* device, Window* window, vk::Renderpass* renderpass);

		static void NewFrame();

		static ImTextureID NewTexture(vk::Texture* tex, vk::Sampler* sampler, vk::ImageLayout layout);

		static void Render(vk::CommandList* cmd);

		static void Destroy();

	private:
	};

}*/