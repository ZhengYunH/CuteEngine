#include "Math/Matrix.h"
#include "Common/KeyCodes.h"
#include <wtypes.h>

namespace zyh
{
	class Camera
	{
	public:
		void handleInputKeyDown(UINT key);
		void handleInputKeyUp(UINT key);
		void handleMouseButtonDown(EMOUSE_BUTTON key);
		void handleMouseButtonUp(EMOUSE_BUTTON key);
		void handleMouseWheel(short delta);
		void handleMouseMove(int32_t x, int32_t y);

	private:
		Vector3 mPosition_{ 0,0,0 };
		Vector3 mEuler_{ 0,0,0 };
		Matrix4x4 mViewMatrix_;
	};
}

