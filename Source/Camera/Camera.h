#pragma once
#include "Math/Matrix4x3.h"
#include "Math/Matrix4x4.h"
#include "Math/MathUtil.h"
#include "Common/KeyCodes.h"
#include "Core/InputSystem.h"
#include "Core/Engine.h"
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
			
			BindInputEvent(KeyDown, *this, Camera::EventKeyDown);
			BindInputEvent(KeyUp, *this, Camera::EventKeyUp);

			BindInputEvent(LeftMouseDown, *this, Camera::EventLeftMouseDown);
			BindInputEvent(RightMouseDown, *this, Camera::EventRightMouseDown);
			BindInputEvent(MidMouseDown, *this, Camera::EventMidMouseDown);
			BindInputEvent(LeftMouseUp, *this, Camera::EventLeftMouseUp);
			BindInputEvent(RightMouseUp, *this, Camera::EventRightMouseUp);
			BindInputEvent(MidMouseUp, *this, Camera::EventMidMouseUp);
			BindInputEvent(MouseMove, *this, Camera::EventMouseMove);
			BindInputEvent(MouseWheel, *this, Camera::EventMouseWheel);
		}

	public:
		/// Event Binding
 		void EventKeyDown(KEY_TYPE key) { this->handleInputKeyDown(key); }
		void EventKeyUp(KEY_TYPE key) { this->handleInputKeyUp(key); }
		void EventLeftMouseDown(KEY_TYPE x, KEY_TYPE y) { this->handleMouseButtonDown(LEFT, x, y); }
		void EventRightMouseDown(KEY_TYPE x, KEY_TYPE y) { this->handleMouseButtonDown(RIGHT, x, y); }
		void EventMidMouseDown(KEY_TYPE x, KEY_TYPE y) { this->handleMouseButtonDown(MID, x, y); }
		void EventLeftMouseUp(KEY_TYPE x, KEY_TYPE y) { this->handleMouseButtonUp(LEFT, x, y); }
		void EventRightMouseUp(KEY_TYPE x, KEY_TYPE y) { this->handleMouseButtonUp(RIGHT, x, y); }
		void EventMidMouseUp(KEY_TYPE x, KEY_TYPE y) { this->handleMouseButtonUp(MID, x, y); }
		void EventMouseWheel(short delta) { this->handleMouseWheel(delta); }
		void EventMouseMove(KEY_TYPE x, KEY_TYPE y) { this->handleMouseMove(x, y, GEngine->GetDeltaTime()); }

	private:
		void handleInputKeyDown(WPARAM key);
		void handleInputKeyUp(WPARAM key);
		void handleMouseButtonDown(EMOUSE_BUTTON key, int32_t x, int32_t y);
		void handleMouseButtonUp(EMOUSE_BUTTON key, int32_t x, int32_t y);
		void handleMouseWheel(short delta);
		void handleMouseMove(int32_t x, int32_t y, float deltaTime);

	public:
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
		float mMoveSpeed_{ 0.1f };
		
	public:
		float mScreenHeight_{ -1.0f };
		float mScreenWidth_{ -1.0f };
	};
}

