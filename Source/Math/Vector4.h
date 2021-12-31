#pragma once
#include "MathUtil.h"


namespace zyh
{
	class Vector4
	{
	public: // member
		float x{ 0.0f }, y{ 0.0f }, z{ 0.f }, w{ 0.f };
	public: // ctor
		constexpr Vector4() noexcept = default;
		constexpr Vector4(const Vector4&) noexcept = default;
		constexpr Vector4(float nx, float ny, float nz, float nw) noexcept : x(nx), y(ny), z(nz), w(nw) {}
		constexpr Vector4(float* arr) noexcept : x(arr[0]), y(arr[1]), z(arr[2]), w(arr[3]) {}

	public: // method
		void SetZero() { x = y = z = 0.0f; }
		static constexpr Vector4 GetZero() { return Vector4(0.f, 0.f, 0.f, 0.f); }
		inline float GetLengthSquared() const { return x * x + y * y + z * z + w * w; }
		float GetLength() const { return Sqrt(GetLengthSquared()); }
		void Normalize()
		{
			float magSqr = GetLengthSquared();
			if (magSqr > 0.0f)
			{
				float oneOverMag = 1.0f / Sqrt(magSqr);
				x *= oneOverMag;
				y *= oneOverMag;
				z *= oneOverMag;
				w *= oneOverMag;
			}
		}
		Vector4 GetNormalized()
		{
			Vector4 v(*this);
			v.Normalize();
			return v;
		}

	public: //operator
		Vector4& operator =(const Vector4 & rhs) { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }
		bool operator ==(const Vector4 & rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
		Vector4 operator -() const { return Vector4(-x, -y, -z, -w); }
		Vector4 operator +(const Vector4 & rhs) const { return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
		Vector4 operator -(const Vector4 & rhs) const { return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
		Vector4 operator *(float scaler) const { return Vector4(x * scaler, y * scaler, z * scaler, w * scaler); }
		float operator *(const Vector4& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w; }
		Vector4 operator /(float scaler) const { return Vector4(x / scaler, y / scaler, z / scaler, w / scaler); }
		Vector4 operator +=(const Vector4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
		Vector4 operator -=(const Vector4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
		constexpr Vector4 operator *=(float scaler) { x *= scaler; y *= scaler; z *= scaler; w *= scaler; return *this; }
		Vector4 operator /=(float scaler) { x /= scaler; y /= scaler; z /= scaler; w /= scaler; return *this; }
	};
	inline Vector4 operator*(float k, const Vector4& v) { return v * k; }
}