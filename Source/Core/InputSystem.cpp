#include "InputSystem.h"

// Define a Helper Macro To Handle Message
#define BOARDCAST_EVENT(Msg, OpType, ...)\
case Msg: \
	INPUT_SYSTEM_OP_EVENT(OpType).BoardCast(##__VA_ARGS__); \
	break; 

namespace zyh
{
	InputSystem* GInputSystem = new InputSystem();

	void InputSystem::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
			// Common Event
			BOARDCAST_EVENT(WM_CLOSE, Close);
			BOARDCAST_EVENT(WM_SIZE, Size, wParam == SIZE_MINIMIZED, LOWORD(lParam), HIWORD(lParam));
			BOARDCAST_EVENT(WM_ENTERSIZEMOVE, EnterResizeMove);
			BOARDCAST_EVENT(WM_EXITSIZEMOVE, ExitResizeMove);

			// Keyboard Event
			BOARDCAST_EVENT(WM_KEYDOWN, KeyDown, wParam);
			BOARDCAST_EVENT(WM_KEYUP, KeyUp, wParam);

			// Mouse Event
			BOARDCAST_EVENT(WM_LBUTTONDOWN, LeftMouseDown, LOWORD(lParam), HIWORD(lParam));
			BOARDCAST_EVENT(WM_RBUTTONDOWN, RightMouseDown, LOWORD(lParam), HIWORD(lParam));
			BOARDCAST_EVENT(WM_MBUTTONDOWN, MidMouseDown, LOWORD(lParam), HIWORD(lParam));
			BOARDCAST_EVENT(WM_LBUTTONUP, LeftMouseUp, LOWORD(lParam), HIWORD(lParam));
			BOARDCAST_EVENT(WM_RBUTTONUP, RightMouseUp, LOWORD(lParam), HIWORD(lParam));
			BOARDCAST_EVENT(WM_MBUTTONUP, MidMouseUp, LOWORD(lParam), HIWORD(lParam));
			BOARDCAST_EVENT(WM_MOUSEWHEEL, MouseWheel, GET_WHEEL_DELTA_WPARAM(wParam));
			BOARDCAST_EVENT(WM_MOUSEMOVE, MouseMove, LOWORD(lParam), HIWORD(lParam));

		default:
			break;
		}
	}
}

#undef BOARDCAST_EVENT