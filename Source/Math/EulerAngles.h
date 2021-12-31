#pragma once
#include "MathUtil.h"


namespace zyh
{
	class Quaternion;
	class Matrix4x3;
	class Matrix3x3;


	class EulerAngles 
	{
	public:
		static const EulerAngles IDENTITY;

	public: // member
		float pitch;
		float yaw;
		float roll;

	public: // ctor
		EulerAngles() noexcept = default;
		EulerAngles(const EulerAngles& rhs) noexcept : pitch(rhs.pitch), yaw(rhs.yaw), roll(rhs.roll) {}
		EulerAngles(float p, float y, float r) noexcept : pitch(p), yaw(y), roll(r) {}
		EulerAngles(const Quaternion& quaterion) noexcept;
		EulerAngles(const Matrix4x3& mat) noexcept;
		EulerAngles(const Matrix3x3& mat) noexcept;

	public: // member function
		void identity() { pitch = yaw = roll = 0.0f; }
		
		void canonizer();
	};

	const EulerAngles EulerAngles::IDENTITY(0.f, 0.f, 0.f);

	void EulerAngles::canonizer()
	{
		pitch = WrapPI(pitch);

		if (pitch < -MATH_HALF_PI)
		{
			pitch = -MATH_PI - pitch;
			yaw += MATH_PI;
			roll += MATH_PI;
		}
		else if (pitch > MATH_HALF_PI)
		{
			pitch = MATH_PI - pitch;
			yaw += MATH_PI;
			roll += MATH_PI;
		}

		if (Fabs(pitch) > MATH_HALF_PI - EPISILON)
		{
			yaw += roll;
			roll = 0.f;
		}
		else
		{
			roll = WrapPI(roll);
		}

		yaw = WrapPI(yaw);
	}

}