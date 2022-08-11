#pragma once
#include "Common/Config.h"


namespace zyh
{
	class ClientScene;

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

	private:
		typedef std::chrono::steady_clock::time_point TimeType;
		TimeType mCurrFrameTime_;
		TimeType mLastFrameTime_;
		float mDeltaTime_{ 0.033f };
		uint64_t mCurrFrame_{ 0 };

	public:
		inline bool IsFirstFrame() const { return mCurrFrame_ == 0; }
		uint64_t GetCurrFrame() { return mCurrFrame_; }

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