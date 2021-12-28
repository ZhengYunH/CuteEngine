#include "Camera.h"

namespace zyh
{

	void Camera::handleInputKeyDown(UINT key)
	{
		float deltaTime = 0.033f;

		switch (key)
		{
		case KEY_W:
			mPosition_ = mPosition_ + mPosition_.z_axis() * deltaTime;
			break;
		case KEY_S:
			mPosition_ = mPosition_ - mPosition_.z_axis() * deltaTime;
			break;
		case KEY_A:
			mPosition_ = mPosition_ - mPosition_.x_axis() * deltaTime;
			break;
		case KEY_D:
			mPosition_ = mPosition_ + mPosition_.x_axis() * deltaTime;
			break;
		default:
			break;
		}
	}

	void Camera::handleInputKeyUp(UINT key)
	{

	}

	void Camera::handleMouseButtonDown(EMOUSE_BUTTON key)
	{
		switch (key)
		{
		case LEFT:
			break;
		case MID:
			break;
		case RIGHT:
			break;
		default:
			assert(0);
			break;
		}
	}

	void Camera::handleMouseButtonUp(EMOUSE_BUTTON key)
	{
		switch (key)
		{
		case LEFT:
			break;
		case MID:
			break;
		case RIGHT:
			break;
		default:
			assert(0);
			break;
		}
	}

	void Camera::handleMouseWheel(short delta)
	{

	}

	void Camera::handleMouseMove(int32_t x, int32_t y)
	{

	}

}
