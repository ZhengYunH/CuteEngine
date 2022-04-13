#pragma once
#include "Math/Matrix4x3.h"
#include "Math/Matrix4x4.h"
#include "Math/MathUtil.h"
#include "Common/KeyCodes.h"
#include <wtypes.h>
#include <cstdint>


namespace zyh
{
	enum ECAMERA_KEY_BIT : uint64_t
	{
		EC_KEY_NONE = 0,
		EC_KEY_W = 1 << 1,
		EC_KEY_S = 1 << 2,
		EC_KEY_A = 1 << 3,
		EC_KEY_D = 1 << 4,
		EC_KEY_Q = 1 << 5,
		EC_KEY_E = 1 << 6,
	};

	enum ECAMEAR_MOUSE_BUTTON : char
	{
		EC_MOUSE_NONE = 0,
		EC_MOUSE_LEFT = 1 << 1,
		EC_MOUSE_MID = 1 << 2,
		EC_MOUSE_RIGHT = 1 << 3
	};

	class Camera
	{
	public:
		Camera() noexcept
		{
			mTransform_.SetIdentity();
			updateProjMatrix();
		}

		void handleInputKeyDown(WPARAM key);
		void handleInputKeyUp(WPARAM key);
		void handleMouseButtonDown(EMOUSE_BUTTON key, int32_t x, int32_t y);
		void handleMouseButtonUp(EMOUSE_BUTTON key, int32_t x, int32_t y);
		void handleMouseWheel(short delta);
		void handleMouseMove(int32_t x, int32_t y, float deltaTime);

		void tick(float deltaTime);
		void setTransform(const Matrix4x3& mat) { mTransform_ = mat; }
		void reset() { 
			mTransform_.SetIdentity(); 
			mFov_ = 45.f;
		}

	public:
		Matrix4x3 getViewMatrix() const { return mTransform_.GetInverse(); }
		const Matrix4x4& getProjMatrix() const { return mProjMatrix_; }

	public:
		void updateProjMatrix();

	private:
		Matrix4x3 mTransform_;
		Matrix4x4 mProjMatrix_;

	private:
		uint64_t mPressingKeyBit_{ 0 };
		char mPressingMouseKeyBit_{ 0 };
		int32_t mLastMousePositionX, mLastMousePositionY;
		float mFov_{ 45.f };
		float mNear_{ 0.01f };
		float mFar_{ 5000.f };
		float mMoveSpeed_{ 0.01f };
		
	public:
		float mScreenHeight_{ -1.0f };
		float mScreenWidth_{ -1.0f };
	};
}

