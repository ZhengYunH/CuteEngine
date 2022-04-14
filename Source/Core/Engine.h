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
		inline const float GetDeltaTime() const { return mDeltaTime_; }

	private:
		void Initialize();
		void Tick();
		void CleanUp();
		
	public:
		ClientScene* Scene;
		VulkanBase* Vulkan;

	private:
		// TODO 
		float mDeltaTime_{ 0.033f };
	};

	extern Engine* GEngine;
	extern ClientScene* GClientScene;
}