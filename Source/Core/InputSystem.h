#pragma once
#include "Common/Config.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace zyh
{

	class InputSystem
	{
		friend class Engine;

	private:
		void HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	};

	extern InputSystem* GInputSystem;
}