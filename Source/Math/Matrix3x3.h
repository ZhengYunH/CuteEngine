#pragma once
#include "Common/Config.h"
#include "MathUtil.h"
#include "Quaternion.h"


namespace zyh
{
	class Matrix4x3;

	class Matrix3x3
	{
	private:
		struct as_array
		{
			float m3x3[3][3];
		};
		constexpr inline as_array& __array() noexcept
		{
			return *(as_array*)this;
		}
		constexpr inline const as_array& __array() const noexcept
		{
			return *(const as_array*)this;
		}
	
	public:
		float m00{ 0.0f }, m01{ 0.0f }, m02{ 0.0f };
		float m10{ 0.0f }, m11{ 0.0f }, m12{ 0.0f };
		float m20{ 0.0f }, m21{ 0.0f }, m22{ 0.0f };

		// Accessible member.
		static constexpr int DIMENSION = 9;
		static constexpr int ROW = 3;
		static constexpr int COL = 3;

		inline const float& operator()(uint32 row, uint32 col) const noexcept
		{
			HYBRID_CHECK(row < ROW&& col < COL);
			return __array().m3x3[row][col];
		}
		inline float& operator()(uint32 row, uint32 col) noexcept
		{
			HYBRID_CHECK(row < ROW&& col < COL);
			return __array().m3x3[row][col];
		}

	public:
		constexpr inline Matrix3x3() noexcept = default;
		constexpr inline Matrix3x3(const Matrix3x3&) noexcept = default;
		constexpr inline Matrix3x3& operator=(const Matrix3x3&) noexcept = default;
		constexpr inline Matrix3x3(const Quaternion& q) noexcept
		{
			Quaternion nq = q.GetNormalized();
			const float x2 = nq.x + nq.x;
			const float y2 = nq.y + nq.y;
			const float z2 = nq.z + nq.z;
			const float xx = nq.x * x2;
			const float xy = nq.x * y2;
			const float xz = nq.x * z2;
			const float yy = nq.y * y2;
			const float yz = nq.y * z2;
			const float zz = nq.z * z2;
			const float wx = nq.w * x2;
			const float wy = nq.w * y2;
			const float wz = nq.w * z2;
			m00 = 1.0f - (yy + zz); m01 = xy + wz;          m02 = xz - wy;
			m10 = xy - wz;          m11 = 1.0f - (xx + zz); m12 = yz + wx;
			m20 = xz + wy;          m21 = yz - wx;          m22 = 1.0f - (xx + yy);
		}
		constexpr inline Matrix3x3(
			float in00, float in01, float in02,
			float in10, float in11, float in12,
			float in20, float in21, float in22
		) noexcept
			: m00{ in00 }, m01{ in01 }, m02{ in02 }
			, m10{ in10 }, m11{ in11 }, m12{ in12 }
			, m20{ in20 }, m21{ in21 }, m22{ in22 }
		{
		}
		constexpr inline Matrix3x3(
			const Vector3& inX, 
			const Vector3& inY, 
			const Vector3& inZ
		) noexcept
		{
			m00 = inX.x; m01 = inX.y; m02 = inX.z;
			m10 = inY.x; m11 = inY.y; m12 = inY.z;
			m20 = inZ.x; m21 = inZ.y; m22 = inZ.z;
		}
	public:
		inline void SetIdentity() noexcept
		{
			m00 = 1; m01 = 0; m02 = 0;
			m10 = 0; m11 = 1; m12 = 0;
			m20 = 0; m21 = 0; m22 = 1;
		};
		inline void SetTranspose() noexcept
		{
			std::swap(m01, m10);
			std::swap(m02, m20);
			std::swap(m12, m21);
		}
		constexpr inline Matrix3x3 GetTransposed() const noexcept
		{
			Matrix3x3 out;
			out.m00 = m00;
			out.m01 = m10;
			out.m02 = m20;

			out.m10 = m01;
			out.m11 = m11;
			out.m12 = m21;

			out.m20 = m02;
			out.m21 = m12;
			out.m22 = m22;
			return out;
		}
		constexpr inline float Determinant() const noexcept
		{
			return (m00 * m11 * m22) + (m01 * m12 * m20) + (m02 * m10 * m21) - (m02 * m11 * m20) - (m00 * m12 * m21) - (m01 * m10 * m22);
		}
		inline void SetRotationX(const float rad) noexcept
		{
			auto [s, c] = SinCos(rad);
			m00 = 1.0f; m01 = 0.0f; m02 = 0.0f;
			m10 = 0.0f; m11 = c;    m12 = s;
			m20 = 0.0f; m21 = -s;   m22 = c;
		}
		inline void SetRotationY(const float rad) noexcept
		{
			auto [s, c] = SinCos(rad);
			m00 = c;    m01 = 0.0f; m02 = -s;
			m10 = 0.0f; m11 = 1.0f; m12 = 0.0f;
			m20 = s;    m21 = 0.0f; m22 = c;
		}
		inline void SetRotationZ(const float rad) noexcept
		{
			auto [s, c] = SinCos(rad);
			m00 = c;    m01 = s;    m02 = 0.0f;
			m10 = -s;   m11 = c;    m12 = 0.0f;
			m20 = 0.0f; m21 = 0.0f; m22 = 1.0f;
		}
		inline const Vector3& GetXAxis() const noexcept
		{
			return *((Vector3*)(__array().m3x3[0]));
		}
		inline const Vector3& GetYAxis() const noexcept
		{
			return *((Vector3*)(__array().m3x3[1]));
		}
		inline const Vector3& GetZAxis() const noexcept
		{
			return *((Vector3*)(__array().m3x3[2]));
		}
		constexpr inline void SetXAxis(const Vector3& v) noexcept
		{
			m00 = v.x;
			m01 = v.y;
			m02 = v.z;
		}
		constexpr inline void SetYAxis(const Vector3& v) noexcept
		{
			m10 = v.x;
			m11 = v.y;
			m12 = v.z;
		}
		constexpr inline void SetZAxis(const Vector3& v) noexcept
		{
			m20 = v.x;
			m21 = v.y;
			m22 = v.z;
		}
		inline Vector3 GetScale() const noexcept
		{
			return Vector3{
				GetXAxis().GetLength(),
					GetYAxis().GetLength(),
					GetZAxis().GetLength(),
			};
		}
		inline float GetPitch() const noexcept
		{
			return ATan2(-m21, Sqrt(m20 * m20 + m22 * m22));
		}
		inline float GetYaw() const noexcept
		{
			return std::atan2(m20, m22);
		}
		inline float GetRoll() const noexcept
		{
			Vector3 scale = GetScale();
			return std::atan2(m01 * scale.y, m11 * scale.x);
		}
		inline void SetPitchYawRoll(const float& p, const float& y, const float& r) noexcept
		{
			Vector3 scale = GetScale();
			auto [sy, cy] = SinCos(y);
			auto [sp, cp] = SinCos(p);
			auto [sr, cr] = SinCos(r);
			m00 = scale.x * (sr * sp * sy + cr * cy);
			m01 = scale.x * (sr * cp);
			m02 = scale.x * (sr * sp * cy - cr * sy);
			m10 = scale.y * (cr * sp * sy - sr * cy);
			m11 = scale.y * (cr * cp);
			m12 = scale.y * (cr * sp * cy + sr * sy);
			m20 = scale.z * (cp * sy);
			m21 = scale.z * (-sp);
			m22 = scale.z * (cp * cy);
		}
		constexpr inline void SetScaleMatrix(const Vector3& s) noexcept
		{
			m00 = s.x; m01 = 0;   m02 = 0;
			m10 = 0;   m11 = s.y; m12 = 0;
			m20 = 0;   m21 = 0;   m22 = s.z;
		}
		inline void SetScale(const Vector3& s) noexcept
		{
			((Vector3*)(__array().m3x3[0]))->Normalize();
			(*((Vector3*)(__array().m3x3[0]))) *= s.x;
			((Vector3*)(__array().m3x3[1]))->Normalize();
			(*((Vector3*)(__array().m3x3[1]))) *= s.y;
			((Vector3*)(__array().m3x3[2]))->Normalize();
			(*((Vector3*)(__array().m3x3[2]))) *= s.z;
		}
		constexpr inline void SetInvert() noexcept
		{
			*this = GetInverse();
		}
		constexpr inline Matrix3x3 GetInverse() const noexcept
		{
			Matrix3x3 m;
			m.m00 = m22 * m11 - m21 * m12;
			m.m01 = m21 * m02 - m22 * m01;
			m.m02 = m01 * m12 - m02 * m11;
			m.m10 = m20 * m12 - m22 * m10;
			m.m11 = m22 * m00 - m20 * m02;
			m.m12 = m02 * m10 - m00 * m12;
			m.m20 = m21 * m10 - m20 * m11;
			m.m21 = m20 * m01 - m21 * m00;
			m.m22 = m00 * m11 - m01 * m10;

			float det = (m.m00 * m00 + m.m01 * m10 + m.m02 * m20);
			if (IsZero(det))
			{
				return Matrix3x3{
					Vector3{ 1, 0, 0 },
					Vector3{ 0, 1, 0 },
					Vector3{ 0, 0, 1 },
				};
			}
			float idet = 1.0f / det;
			m.m00 *= idet; m.m01 *= idet; m.m02 *= idet;
			m.m10 *= idet; m.m11 *= idet; m.m12 *= idet;
			m.m20 *= idet; m.m21 *= idet; m.m22 *= idet;
			return m;
		}
		constexpr inline Vector3 TransformVector(const Vector3& v) const noexcept
		{
			return Vector3{
				v.x* m00 + v.y * m10 + v.z * m20,
				v.x* m01 + v.y * m11 + v.z * m21,
				v.x* m02 + v.y * m12 + v.z * m22,
			};
		}
		constexpr void OrthonormalizeFast() noexcept
		{
			Vector3 x{ m00, m01, m02, };
			Vector3 y{ m10, m11, m12, };
			Vector3 z;
			x = x.GetNormalized();
			z = (x ^ y).GetNormalized();
			y = (z ^ x).GetNormalized();
			m00 = x.x; m01 = x.y; m02 = x.z;
			m10 = y.x; m11 = y.y; m12 = y.z;
			m20 = z.x; m21 = z.y; m22 = z.z;
		}
		constexpr inline Vector3 GetColumn3(int i) const noexcept
		{
			HYBRID_CHECK(i < COL);
			return Vector3{
				__array().m3x3[0][i],
				__array().m3x3[1][i],
				__array().m3x3[2][i],
			};
		}
		constexpr inline const Vector3& GetRow3(int i) const noexcept
		{
			HYBRID_CHECK(i < ROW);
			return *(Vector3*)__array().m3x3[i];
		}
		constexpr inline bool IsIdentity() const noexcept
		{
			return 0 == (Fabs(1.f - m00) + Fabs(m01) + Fabs(m02) + Fabs(m10) + Fabs(1.f - m11) + Fabs(m12) + Fabs(m20) + Fabs(m21) + Fabs(1.f - m22));
		}
		constexpr inline int IsOrthonormal(float threshold = 0.001) const noexcept
		{
			float d0 = Fabs(GetColumn3(0) * GetColumn3(1));
			if (d0 > threshold)
				return 0;
			float d1 = Fabs(GetColumn3(0) * GetColumn3(2));
			if (d1 > threshold)
				return 0;
			float d2 = Fabs(GetColumn3(1) * GetColumn3(2));
			if (d2 > threshold)
				return 0;
			int a = (Fabs(1 - (GetColumn3(0) * GetColumn3(0)))) < threshold;
			int b = (Fabs(1 - (GetColumn3(1) * GetColumn3(1)))) < threshold;
			int c = (Fabs(1 - (GetColumn3(2) * GetColumn3(2)))) < threshold;
			return a & b & c;
		}
		inline bool operator==(const Matrix3x3& m) const
		{
			return memcmp(this, &m, sizeof(*this)) == 0;
		}
	};
	static constexpr Matrix3x3 Matrix3x3_Identity
	{
		1.f, 0.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 0.f, 1.f
	};
	static constexpr Matrix3x3 Matrix3x3_Zero
	{
		0.f, 0.f, 0.f,
		0.f, 0.f, 0.f,
		0.f, 0.f, 0.f
	};
}