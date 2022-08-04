#include "Engine.h"
#include "Graphics/Vulkan/VulkanBase.h"
#include "ClientScene.h"
#include "Graphics/Common/Renderer.h"
#include "InputSystem.h"


namespace zyh
{
	Engine* GEngine = new Engine();

	Engine::Engine()
	{
		Scene = new ClientScene();
	}

	Engine::~Engine()
	{
		delete Scene;
	}

	void Engine::Run()
	{
		Initialize();
		while (true)
		{
			Tick();
		}
		CleanUp();
	}

	void Engine::Initialize()
	{
		mCurrFrameTime_ = mLastFrameTime_ = std::chrono::high_resolution_clock::now();

		InitializeWindow();
		Scene->Initialize();
	}

	void Engine::Tick()
	{
		// Update Timer First
		mCurrFrameTime_ = std::chrono::high_resolution_clock::now();
		mDeltaTime_ = std::chrono::duration<float, std::chrono::seconds::period>(mCurrFrameTime_ - mLastFrameTime_).count();
		mLastFrameTime_ = mCurrFrameTime_;

		// Tick Logic Scene
		Scene->Tick();

		mCurrFrame_ += 1;
	}

	void Engine::CleanUp()
	{
		Scene->CleanUp();
	}
	
#if defined(_WIN32)
	LRESULT Engine::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (GInputSystem)
			GInputSystem->HandleMessage(hWnd, uMsg, wParam, lParam);
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}


	HWND Engine::InitializeWindow()
	{
		mWindowInstance_ = GetModuleHandle(0);

		WNDCLASSEX wndClass;

		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = WndProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = mWindowInstance_;
		wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = tools::stringToLPCWSTR(Setting::AppTitle);
		wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

		if (!RegisterClassEx(&wndClass))
		{
			std::cout << "Could not register window class!\n";
			fflush(stdout);
			exit(1);
		}

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		if (Setting::IsFullscreen)
		{
			if ((Setting::AppWidth != (uint32_t)screenWidth) && (Setting::AppHeight != (uint32_t)screenHeight))
			{
				DEVMODE dmScreenSettings;
				memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmPelsWidth = Setting::AppWidth;
				dmScreenSettings.dmPelsHeight = Setting::AppHeight;
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				{
					if (MessageBox(NULL, L"Full Screen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					{
						Setting::IsFullscreen = false;
					}
					else
					{
						return nullptr;
					}
				}
				screenWidth = Setting::AppWidth;
				screenHeight = Setting::AppHeight;
			}
		}

		DWORD dwExStyle;
		DWORD dwStyle;

		if (Setting::IsFullscreen)
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}
		else
		{
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}

		RECT windowRect;
		windowRect.left = 0L;
		windowRect.top = 0L;
		windowRect.right = Setting::IsFullscreen ? (long)screenWidth : (long)Setting::AppWidth;
		windowRect.bottom = Setting::IsFullscreen ? (long)screenHeight : (long)Setting::AppHeight;

		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

		std::string windowTitle = Setting::AppTitle;
		mWindow_ = CreateWindowEx(0,
			tools::stringToLPCWSTR(windowTitle),
			tools::stringToLPCWSTR(windowTitle),
			dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0,
			0,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			mWindowInstance_,
			NULL);

		if (!Setting::IsFullscreen)
		{
			// Center on screen
			uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
			uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
			SetWindowPos(mWindow_, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}

		if (!mWindow_)
		{
			printf("Could not create window!\n");
			fflush(stdout);
			return nullptr;
		}

		ShowWindow(mWindow_, SW_SHOW);
		SetForegroundWindow(mWindow_);
		SetFocus(mWindow_);
		return mWindow_;
	}
#endif
}
