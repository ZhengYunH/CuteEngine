#include "Math/Matrix.h"
#include "KeyCodes.h"

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
	Matrix4x4 mViewMatrix_;
};