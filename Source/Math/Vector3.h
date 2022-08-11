#pragma once
#include "MathUtil.h"


namespace zyh
{
	class Vector3
	{
	public: // static method
		static float dot(const Vector3& lhs, const Vector3& rhs) 
		{
			return lhs * rhs;
		}
		static Vector3 cross(const Vector3& lhs, const Vector3& rhs)
		{
			return Vector3(
				lhs.y * rhs.z - lhs.z * rhs.y,
				lhs.z * rhs.x - lhs.x * rhs.z,
				lhs.x * rhs.y - lhs.y * rhs.x
			);
		}
		static float distance(const Vector3& lhs, const Vector3& rhs)
		{
			float dx = lhs.x - rhs.x;
			float dy = lhs.y - rhs.y;
			float dz = lhs.z - rhs.z;
			return std::sqrt(dx * dx + dy * dy + dz * dz);
		}

	public: // member
		float x{ 0.0f }, y{ 0.0f }, z{ 0.f };

	public: // ctor
		constexpr Vector3() noexcept = default;
		constexpr Vector3(const Vector3&) noexcept = default;
		constexpr Vector3(float nx, float ny, float nz) noexcept : x(nx), y(ny), z(nz) {}
		constexpr Vector3(float* arr) noexcept : x(arr[0]), y(arr[1]), z(arr[2]) {}
		constexpr Vector3(float n) noexcept : x(n), y(n), z(n) {}

	public: // method
		void SetZero() { x = y = z = 0.0f; }
		static constexpr Vector3 GetZero() { return Vector3(0.f, 0.f, 0.f); }
		constexpr inline float GetLengthSquared() const { return x * x + y * y + z * z; }
		float GetLength() const { return Sqrt(GetLengthSquared()); }
		constexpr void Normalize()
		{
			float magSqr = GetLengthSquared();
			if (magSqr > 0.0f)
			{
				float oneOverMag = 1.0f / Sqrt(magSqr);
				x *= oneOverMag;
				y *= oneOverMag;
				z *= oneOverMag;
			}
		}
		Vector3 GetNormalized() 
		{
			Vector3 v(*this);
			v.Normalize();
			return v;
		}

	public: //operator
		constexpr Vector3& operator =(const Vector3& rhs){ x = rhs.x; y = rhs.y; z = rhs.z; return *this; }
		bool operator ==(const Vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
		Vector3 operator -() const { return Vector3(-x, -y, -z); }
		Vector3 operator +(const Vector3& rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }
		Vector3 operator -(const Vector3& rhs) const { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }
		Vector3 operator *(float scaler) const { return Vector3(x * scaler, y * scaler, z * scaler); }
		Vector3 operator ^(const Vector3& rhs) { return Vector3::cross(*this, rhs); } // cross product
		float operator *(const Vector3& rhs) const { return Vector3::dot(*this, rhs); } // dot product
		Vector3 operator /(float scaler) const { return Vector3(x / scaler, y / scaler, z / scaler); }
		Vector3 operator +=(const Vector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
		Vector3 operator -=(const Vector3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
		constexpr Vector3 operator *=(float scaler) { x *= scaler; y *= scaler; z *= scaler; return *this; }
		Vector3 operator /=(float scaler) { x /= scaler; y /= scaler; z /= scaler; return *this; }
	};
	inline Vector3 operator*(float k, const Vector3& v) { return v * k; }
}