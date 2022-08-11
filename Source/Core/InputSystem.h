#pragma once
#include "Common/Config.h"
#include "Common/KeyCodes.h"
#include "Core/EventHelper.h"

#define INPUT_SYSTEM_OP_EVENT(OPType) OPType##_Event
#define DEFINE_OP_EVENT(OPType, ...) Event<void, ##__VA_ARGS__> INPUT_SYSTEM_OP_EVENT(OPType)


namespace zyh
{
	class InputSystem
	{
		friend class Engine;

	public:
		// Common Event
		DEFINE_OP_EVENT(Close);
		DEFINE_OP_EVENT(Size, bool/* Is Minimized */, KEY_TYPE/* Width */, KEY_TYPE/* Height */);
		DEFINE_OP_EVENT(EnterResizeMove);
		DEFINE_OP_EVENT(ExitResizeMove);

		// Keyboard Event
		DEFINE_OP_EVENT(KeyDown, KEY_TYPE/* Key Code */);
		DEFINE_OP_EVENT(KeyUp, KEY_TYPE/* Key Code */);
		
		// Mouse Event
		DEFINE_OP_EVENT(LeftMouseDown, KEY_TYPE/* XPosition */, KEY_TYPE/* YPosition */);
		DEFINE_OP_EVENT(RightMouseDown, KEY_TYPE/* XPosition */, KEY_TYPE/* YPosition */);
		DEFINE_OP_EVENT(MidMouseDown, KEY_TYPE/* XPosition */, KEY_TYPE/* YPosition */);
		DEFINE_OP_EVENT(LeftMouseUp, KEY_TYPE/* XPosition */, KEY_TYPE/* YPosition */);
		DEFINE_OP_EVENT(RightMouseUp, KEY_TYPE/* XPosition */, KEY_TYPE/* YPosition */);
		DEFINE_OP_EVENT(MidMouseUp, KEY_TYPE/* XPosition */, KEY_TYPE/* YPosition */);
		DEFINE_OP_EVENT(MouseWheel, KEY_TYPE /* Wheel Offset */);
		DEFINE_OP_EVENT(MouseMove, KEY_TYPE/* XPosition */, KEY_TYPE/* YPosition */);

	public:
		void HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

	extern InputSystem* GInputSystem;
}

#define BindInputEvent(OPType, Object, Func) BIND_EVENT(GInputSystem->OPType##_Event, Object, Func)

#undef DEFINE_OP_EVENT