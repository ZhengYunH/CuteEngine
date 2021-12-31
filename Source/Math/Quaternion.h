#pragma once
#include <tuple>
#include "Common/Config.h"
#include "MathUtil.h"
#include "Vector3.h"


/// <summary>
/// class Quaternion
/// for Quaternion math present we have: Quaternion = [cos(theta/2) sin(theta/2) (^n)]
/// in the Quaternion class, we define as following mention:
///		n = (x, y, z)
///		w = cos(theta/2)
/// </summary>

namespace zyh
{
	class Vector3;
	class EulerAngles;
	class Matrix3x3;

	class Quaternion
	{
	public:
		static float Dot(const Quaternion& lhs, const Quaternion& rhs)
		{
			return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
		}
		static Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t)
		{
			if (t <= 0.0f) return q0;
			if (t >= 1.0f) return q1;

			float cosOmega = Dot(q0, q1);
			float q1x = q1.x;
			float q1y = q1.y;
			float q1z = q1.z;
			float q1w = q1.w;

			if (cosOmega < 0.0f)
			{
				q1x = -q1x;
				q1y = -q1y;
				q1z = -q1z;
				q1w = -q1w;
				cosOmega = -cosOmega;
			}

			float k0, k1;
			if (cosOmega > 0.99f)
			{
				k0 = 1.0f - t;
				k1 = t;
			}
			else
			{
				float sinOmega = Sqrt(1.0f - cosOmega * cosOmega);
				float omega = ATan2(sinOmega, cosOmega);
				float oneOverSinOmega = 1.0f / sinOmega;
				k0 = Sin((1.0f - t) * omega) * oneOverSinOmega;
				k1 = Sin(t * omega) * oneOverSinOmega;
			}

			return q0 * k0  + q1 * k1;
		}
		static Quaternion Pow(const Quaternion& quat, float exponent)
		{
			if (Fabs(quat.w) > 1 - EPISILON)
				return quat;

			float alpha = ACos(quat.w);
			float newAlpha = alpha * exponent;
			float mult = Sin(newAlpha) / Sin(alpha);
			return Quaternion(quat.x * mult, quat.y * mult, quat.z * mult, Cos(newAlpha));
		}

	public:
		float x{ 0.f }, y{ 0.f }, z{ 0.f }, w{ 1.f };

	public: // ctor
		constexpr Quaternion() noexcept = default;
		constexpr Quaternion(const Quaternion&) noexcept = default;
		constexpr Quaternion& operator=(const Quaternion&) noexcept = default;

		constexpr Quaternion(float nx, float ny, float nz, float nw) noexcept : x(nx), y(ny), z(nz), w(nw) {}
		constexpr Quaternion(Vector3 imag, float real) noexcept : x(imag.x), y(imag.y), z(imag.z), w(real) {}
		constexpr Quaternion(const Matrix3x3& mat);
		constexpr Quaternion(const EulerAngles& eulerAngle);

	public: // operator
		constexpr Quaternion operator-() { return Quaternion(-x, -y, -z, -w); }
		constexpr Quaternion operator-(const Quaternion& quat) const { return Quaternion(x - quat.x, y - quat.y, z - quat.x, w - quat.w); }
		constexpr Quaternion operator+(const Quaternion& quat) const { return Quaternion(x + quat.x, y + quat.y, z + quat.x, w + quat.w); }
		constexpr Quaternion operator*(const Quaternion& quat) const // cross production
		{ 
			float w = w * quat.w - x * quat.x - y * quat.y - z * quat.z;
			float x = w * quat.x + x * quat.w + y * quat.z - z * quat.y;
			float y = w * quat.y + y * quat.w + z * quat.x - x * quat.z;
			float z = w * quat.z + z * quat.w + x * quat.y - y * quat.x;
			return Quaternion(x, y, z, w);
		}
		constexpr Quaternion operator*(const float scaler) const noexcept { return Quaternion(x * scaler, y * scaler, z * scaler, w * scaler); }
		constexpr Quaternion operator/(const float scaler) const noexcept { return Quaternion(x / scaler, y / scaler, z / scaler, w / scaler); }

	public:
		constexpr Quaternion GetConjugate() const
		{
			return Quaternion(-x, -y, -z, w);
		}

		void SetIdentity()
		{
			x = y = z = 0.0f;
			w = 1.0f;
		}
		static constexpr Quaternion GetIdentity() noexcept
		{
			return Quaternion{ 0, 0, 0, 1 };
		}
		constexpr Quaternion GetInverse() noexcept
		{
			return GetConjugate() / Length();
		}
		constexpr float LengthSqr() const noexcept 
		{
			return x * x + y * y + z * z + w * w;
		}
		constexpr float Length() const noexcept
		{
			return Sqrt(LengthSqr());
		}
		constexpr void Normalize() noexcept
		{
			float len = Length();
			if (len < EPISILON)
			{
				SetIdentity();
			}
			else
			{
				len = 1.f / len;
				w *= len;
				x *= len;
				y *= len;
				z *= len;
			}
		}
		constexpr Quaternion GetNormalized() const noexcept
		{
			Quaternion q = *this;
			q.Normalize();
			return q;
		}
		constexpr bool IsNormalized() const noexcept
		{
			return (Fabs(1.f - LengthSqr()) <= 0.01f);
		}
		float GetAngle() const noexcept
		{
			return 2.0f * ACos(w); // faster, no need to check w
		}
		Vector3 GetAxis() const noexcept
		{
			Vector3 result{ x, y, z };
			return result.GetNormalized();
		}
		void setToRatateAboutX(float radian)
		{
			float thetaOver2 = radian * .5f;
			w = Cos(thetaOver2);
			x = Sin(thetaOver2);
			y = 0.0f;
			z = 0.0f;
		}
		void setToRatateAboutY(float radian)
		{
			float thetaOver2 = radian * .5f;
			w = Cos(thetaOver2);
			x = 0.0f;
			y = Sin(thetaOver2);
			z = 0.0f;
		}
		void setToRatateAboutZ(float radian)
		{
			float thetaOver2 = radian * .5f;
			w = Cos(thetaOver2);
			x = 0.0f;
			y = 0.0f;
			z = Sin(thetaOver2);
		}
		void setToRotateAboutAxis(const Vector3& axis, float radian)
		{
			HYBRID_CHECK(Fabs(axis.GetLength() - 1.0f) < 0.01f);

			float thetaOver2 = radian * 0.5f;
			float sinThetaOver2 = Sin(thetaOver2);

			w = Cos(thetaOver2);
			x = axis.x * sinThetaOver2;
			y = axis.y * sinThetaOver2;
			z = axis.z * sinThetaOver2;
		}
	};
	inline Quaternion operator*(float k, const Quaternion& q) { return q * k; }

}