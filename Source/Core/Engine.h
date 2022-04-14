#pragma once
#if defined(_WIN32)
#include <windows.h>
#endif

namespace zyh
{
	class ClientScene;
	class VulkanBase;

	class Engine
	{
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

	protected:
#if defined(_WIN32)
		HWND SetupWindow();
	public:
		void HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		HWND mWindow_;
		HINSTANCE mWindowInstance_;
#endif
	};

	extern Engine* GEngine;
	extern ClientScene* GClientScene;
}