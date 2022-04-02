#pragma once

namespace zyh
{
	class ClientScene;
	class VulkanBase;

	class Engine
	{
	public:
		Engine();
		~Engine();

	public:
		void Run();

	private:
		void Initialize();
		void Tick();
		void CleanUp();
		
	public:
		ClientScene* Scene;
		VulkanBase* Vulkan;
	};

	extern Engine* GEngine;
	extern ClientScene* GClientScene;
}