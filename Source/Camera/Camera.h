#include "Math/Matrix4x3.h"
#include "Math/Matrix4x4.h"
#include "Math/MathUtil.h"
#include "Common/KeyCodes.h"
#include <wtypes.h>
#include <cstdint>


namespace zyh
{
	class Camera
	{
	public:
		Camera() noexcept
		{
			mViewMatrix_.SetIdentity();
			updateProjMatrix();
		}

		void handleInputKeyDown(WPARAM key);
		void handleInputKeyUp(WPARAM key);
		void handleMouseButtonDown(EMOUSE_BUTTON key, int32_t x, int32_t y);
		void handleMouseButtonUp(EMOUSE_BUTTON key, int32_t x, int32_t y);
		void handleMouseWheel(short delta);
		void handleMouseMove(int32_t x, int32_t y, float deltaTime);

		void tick(float deltaTime);
		void setTransform(const Matrix4x3& mat) { mViewMatrix_ = mat; }
		void reset() { mViewMatrix_.SetIdentity(); }

	public:
		const Matrix4x3& getViewMatrix() const { return mViewMatrix_; }
		const Matrix4x4& getProjMatrix() const { return mProjMatrix_; }

	public:
		void updateProjMatrix();

	private:
		Matrix4x3 mViewMatrix_;
		Matrix4x4 mProjMatrix_;

	private:
		WPARAM mPressingKey_{ 0 };
		EMOUSE_BUTTON mPressingMouseKey_;
		int32_t mLastMousePositionX, mLastMousePositionY;
		float mFov_{ 45.f };
		float mNear_{ 0.05f };
		float mFar_{ 5000.f };
		
	public:
		float mScreenHeight_{ -1.0f };
		float mScreenWidth_{ -1.0f };
	};
}

