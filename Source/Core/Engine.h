#pragma once
#if defined(_WIN32)
#include <windows.h>
#endif

#include "Common/Config.h"


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
		clock_t mCurrFrameTicks_{ 0 };
		clock_t mLastFrameTicks_{ 0 };
		float mDeltaTime_{ 0.033f };
		uint64_t mCurrFrame_{ 0 };

	public:
		inline bool IsFirstFrame() const { return mCurrFrame_ == 0; }

	protected:
#if defined(_WIN32)
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		HWND InitializeWindow();
	public:
		HWND mWindow_;
		HINSTANCE mWindowInstance_;
#endif
	};

	extern Engine* GEngine;
	extern ClientScene* GClientScene;
}