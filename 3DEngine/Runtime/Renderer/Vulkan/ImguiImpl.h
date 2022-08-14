#pragma once



class Window;

namespace vk
{

	class Device;
	class Renderpass;
	class CommandList;
	enum class ImageLayout;
	class Sampler;
	class Texture;

	typedef void* ImTextureID;

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

}