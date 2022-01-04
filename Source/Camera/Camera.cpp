#include "Camera.h"

namespace zyh
{
	void Camera::handleInputKeyDown(WPARAM key)
	{
		mPressingKey_ = key;
	}

	void Camera::handleInputKeyUp(WPARAM key)
	{
		if(mPressingKey_ == key)
			mPressingKey_ = 0;
	}

	void Camera::handleMouseButtonDown(EMOUSE_BUTTON key, int32_t x, int32_t y)
	{
		mPressingMouseKey_ = key;
		mLastMousePositionX = x;
		mLastMousePositionY = y;
	}

	void Camera::handleMouseButtonUp(EMOUSE_BUTTON key, int32_t x, int32_t y)
	{
		if (key == mPressingMouseKey_)
			mPressingMouseKey_ = NONE;
	}

	void Camera::handleMouseWheel(short delta)
	{
		mFov_ = Clamp(mFov_ - delta / 120.f, 5.f, 90.f);
		updateProjMatrix();
	}

	void Camera::handleMouseMove(int32_t x, int32_t y, float deltaTime)
	{
		if (mPressingMouseKey_ != NONE)
		{
			float fixMoveDistance = 0.01f;
			float fixRotateRate = 0.03f;
			float deltaX = float(x - mLastMousePositionX);
			float deltaY = float(y - mLastMousePositionY);
			switch (mPressingMouseKey_)
			{
			case LEFT:
				mViewMatrix_.SetPitchYawRoll(
					Clamp(mViewMatrix_.GetPitch() + fixRotateRate * deltaTime * deltaY, -MATH_HALF_PI + 0.1f, MATH_HALF_PI - 0.1f),
					mViewMatrix_.GetYaw() - fixRotateRate * deltaTime * deltaX,
					mViewMatrix_.GetRoll()
				);
				break;
			case MID:
				mViewMatrix_.SetTranslation(
					  mViewMatrix_.GetTranslation() 
					- mViewMatrix_.GetYAxis() * deltaY * fixMoveDistance 
					+ mViewMatrix_.GetXAxis() * deltaX * fixMoveDistance
				);
				break;
			case RIGHT:
				break;
			default:
				assert(0);
				break;
			}
			mLastMousePositionX = x;
			mLastMousePositionY = y;
		}
	}

	void Camera::tick(float deltaTime)
	{
		if (mPressingKey_)
		{
			float moveSpeed = 0.01f;
			Vector3 zAxis = mViewMatrix_.GetZAxis();
			switch (mPressingKey_)
			{
			case KEY_W:
				mViewMatrix_.SetTranslation(mViewMatrix_.GetTranslation() - mViewMatrix_.GetZAxis() * deltaTime * moveSpeed);
				break;
			case KEY_S:
				mViewMatrix_.SetTranslation(mViewMatrix_.GetTranslation() + mViewMatrix_.GetZAxis() * deltaTime * moveSpeed);
				break;
			case KEY_A:
				mViewMatrix_.SetTranslation(mViewMatrix_.GetTranslation() - mViewMatrix_.GetXAxis() * deltaTime * moveSpeed);
				break;
			case KEY_D:
				mViewMatrix_.SetTranslation(mViewMatrix_.GetTranslation() + mViewMatrix_.GetXAxis() * deltaTime * moveSpeed);
				break;
			default:
				break;
			}
		}
		
	}

	void Camera::updateProjMatrix()
	{
		float rad = DegreeToRadian(mFov_);
		float h = Cos(0.5f * rad) / Sin(0.5f * rad);
		float w = h * mScreenHeight_ / mScreenWidth_;
		mProjMatrix_ = Matrix4x4(
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, mFar_ / (mNear_ - mFar_), -1,
			0, 0, -(mFar_ * mNear_) / (mFar_ - mNear_), 0
		);
	}

}
