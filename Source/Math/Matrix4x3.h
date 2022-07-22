#pragma once
#include "Common/Config.h"
#include "MathUtil.h"
#include "Matrix3x3.h"
#include "Vector3.h"
#include "Vector4.h"


namespace zyh
{
	class Matrix4x3
	{
	private:
		struct as_array
		{
			float m4x3[4][3];
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
		float m00{ 1.0f }, m01{ 0.0f }, m02{ 0.0f };
		float m10{ 0.0f }, m11{ 1.0f }, m12{ 0.0f };
		float m20{ 0.0f }, m21{ 0.0f }, m22{ 1.0f };
		float m30{ 0.0f }, m31{ 0.0f }, m32{ 0.0f };

		static constexpr int DIMENSION = 12;
		static constexpr int ROW = 4;
		static constexpr int COL = 3;

		constexpr inline const float& operator()(uint32 row, uint32 col) const noexcept
		{
			HYBRID_CHECK(row < ROW&& col < COL);
			return __array().m4x3[row][col];
		}
		constexpr inline float& operator()(uint32 row, uint32 col) noexcept
		{
			HYBRID_CHECK(row < ROW&& col < COL);
			return __array().m4x3[row][col];
		}
	public:
		constexpr inline Matrix4x3() noexcept = default;
		constexpr inline Matrix4x3(const Matrix4x3&) noexcept = default;
		constexpr inline Matrix4x3(
			float in00, float in01, float in02,
			float in10, float in11, float in12,
			float in20, float in21, float in22,
			float in30, float in31, float in32
		) noexcept
			: m00{ in00 }, m01{ in01 }, m02{ in02 }
			, m10{ in10 }, m11{ in11 }, m12{ in12 }
			, m20{ in20 }, m21{ in21 }, m22{ in22 }
			, m30{ in30 }, m31{ in31 }, m32{ in32 }
		{
		}
		constexpr inline Matrix4x3& operator=(const Matrix4x3&) noexcept = default;
		constexpr inline explicit Matrix4x3(const Matrix3x3& rhs) noexcept
		{
			m00 = rhs.m00; m01 = rhs.m01; m02 = rhs.m02;
			m10 = rhs.m10; m11 = rhs.m11; m12 = rhs.m12;
			m20 = rhs.m20; m21 = rhs.m21; m22 = rhs.m22;
			m30 = 0.f; m31 = 0.f; m32 = 0.f;
		}
		void SetIdentity()
		{
			m00 = 1; m01 = 0; m02 = 0;
			m10 = 0; m11 = 1; m12 = 0;
			m20 = 0; m21 = 0; m22 = 1;
			m30 = 0; m31 = 0; m32 = 0;
		}
		void ZeroTranslation()
		{
			m30 = 0; m31 = 0; m32 = 0;
		}
		void SetTranslation(const Vector3& translation)
		{
			m30 = translation.x;
			m31 = translation.y;
			m32 = translation.z;
		}
		constexpr inline const Vector3& GetTranslation() const noexcept
		{
			return *((Vector3*)(__array().m4x3[3]));
		}
		constexpr inline const Vector3 GetXAxis() const noexcept
		{
			return *((Vector3*)(__array().m4x3[0]));
		}
		constexpr inline const Vector3 GetYAxis() const noexcept
		{
			return *((Vector3*)(__array().m4x3[1]));
		}
		constexpr inline const Vector3 GetZAxis() const noexcept
		{
			return *((Vector3*)(__array().m4x3[2]));
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
		constexpr inline Vector3 GetScale() const noexcept
		{
			return Vector3{
				GetXAxis().GetLength(),
				GetYAxis().GetLength(),
				GetZAxis().GetLength()
			};
		}
		inline float GetPitch() const noexcept
		{
			return ATan2(-m21, Sqrt(m20 * m20 + m22 * m22));
		}
		inline float GetYaw() const noexcept
		{
			if (UNEXPECTED(IsZero(m20) && IsZero(m22)))
				return ATan2(-m02, m00);
			return ATan2(m20, m22);
		}
		inline float GetRoll() const noexcept
		{
			if (UNEXPECTED(IsZero(m01) && IsZero(m11)))
				return 0;
			Vector3 scale = GetScale();
			return ATan2(m01 * scale.y, m11 * scale.x);
		}
		constexpr inline void SetScale(const Vector3& s) noexcept
		{
			((Vector3*)(__array().m4x3[0]))->Normalize();
			(*((Vector3*)(__array().m4x3[0]))) *= s.x;
			((Vector3*)(__array().m4x3[1]))->Normalize();
			(*((Vector3*)(__array().m4x3[1]))) *= s.y;
			((Vector3*)(__array().m4x3[2]))->Normalize();
			(*((Vector3*)(__array().m4x3[2]))) *= s.z;
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
		constexpr inline void SetTranslationMatrix(const Vector3& t) noexcept
		{
			m00 = 1.f; m01 = 0.f; m02 = 0.f;
			m10 = 0.f; m11 = 1.f; m12 = 0.f;
			m20 = 0.f; m21 = 0.f; m22 = 1.f;
			m30 = t.x; m31 = t.y; m32 = t.z;
		}
		constexpr inline void SetScaleMatrix(const Vector3& s, const Vector3& t) noexcept
		{
			m00 = s.x; m01 = 0.f; m02 = 0.f;
			m10 = 0.f; m11 = s.y; m12 = 0.f;
			m20 = 0.f; m21 = 0.f; m22 = s.z;
			m30 = t.x; m31 = t.y; m32 = t.z;
		}
		inline void SetRotationX(const float rad, const Vector3& t) noexcept
		{
			Matrix3x3 m;
			m.SetRotationX(rad);
			*this = Matrix4x3(m);
			SetTranslation(t);
		}
		inline void SetRotationY(const float rad, const Vector3& t) noexcept
		{
			Matrix3x3 m;
			m.SetRotationY(rad);
			*this = Matrix4x3(m);
			SetTranslation(t);
		}
		inline void SetRotationZ(const float rad, const Vector3& t) noexcept
		{
			Matrix3x3 m;
			m.SetRotationZ(rad);
			*this = Matrix4x3(m);
			SetTranslation(t);
		}
		constexpr inline void SetInvert() noexcept
		{
			*this = GetInverse();
		}
		constexpr inline Matrix4x3 GetInverse() const noexcept
		{
			Matrix4x3 m;
			m.m00 = (m22 * m11 - m21 * m12);
			m.m01 = (m21 * m02 - m22 * m01);
			m.m02 = (m01 * m12 - m02 * m11);
			m.m10 = (m20 * m12 - m22 * m10);
			m.m11 = (m22 * m00 - m20 * m02);
			m.m12 = (m02 * m10 - m00 * m12);
			m.m20 = (m21 * m10 - m20 * m11);
			m.m21 = (m20 * m01 - m21 * m00);
			m.m22 = (m00 * m11 - m01 * m10);
			m.m30 = (m22 * m31 * m10 + m20 * m32 * m11 + m21 * m30 * m12) - (m21 * m32 * m10 + m22 * m30 * m11 + m20 * m31 * m12);
			m.m31 = (m21 * m32 * m00 + m22 * m30 * m01 + m20 * m31 * m02) - (m22 * m31 * m00 + m20 * m32 * m01 + m21 * m30 * m02);
			m.m32 = (m02 * m11 * m30 + m00 * m12 * m31 + m01 * m10 * m32) - (m01 * m12 * m30 + m02 * m10 * m31 + m00 * m11 * m32);
			float det = (m.m00 * m00 + m.m01 * m10 + m.m02 * m20);
			if (IsZero(det))
			{
				return Matrix4x3{
					1, 0, 0,
					0, 1, 0,
					0, 0, 1,
					0, 0, 0
				};
			}
			float idet = float(1.0) / det;
			m.m00 *= idet; m.m01 *= idet; m.m02 *= idet;
			m.m10 *= idet; m.m11 *= idet; m.m12 *= idet;
			m.m20 *= idet; m.m21 *= idet; m.m22 *= idet;
			m.m30 *= idet; m.m31 *= idet; m.m32 *= idet;
			return m;
		}
		constexpr inline Vector4 TransformVector(const Vector4& v) const noexcept
		{
			return Vector4{
				v.x * m00 + v.y * m10 + v.z * m20,
				v.x * m01 + v.y * m11 + v.z * m21,
				v.x * m02 + v.y * m12 + v.z * m22,
				v.w
			};
		}
		constexpr inline Vector3 TransformPoint(const Vector3& v) const noexcept
		{
			return Vector3{
				v.x* m00 + v.y * m10 + v.z * m20 + m30,
				v.x* m01 + v.y * m11 + v.z * m21 + m31,
				v.x* m02 + v.y * m12 + v.z * m22 + m32,
			};
		}
		constexpr inline void OrthonormalizeFast() noexcept
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
				__array().m4x3[0][i],
					__array().m4x3[1][i],
					__array().m4x3[2][i],
			};
		}
		constexpr inline const Vector3& GetRow3(int i) const noexcept
		{
			HYBRID_CHECK(i < ROW);
			return *(Vector3*)__array().m4x3[i];
		}
		constexpr inline void SetRow3(int i, const Vector3& v) noexcept
		{
			HYBRID_CHECK(i < ROW);

			*(Vector3*)__array().m4x3[i] = v;
		}
		constexpr inline float Determinant() const noexcept
		{
			return (m00 * m11 * m22) + (m01 * m12 * m20) + (m02 * m10 * m21) - (m02 * m11 * m20) - (m00 * m12 * m21) - (m01 * m10 * m22);
		}
		constexpr inline bool IsIdentity() const noexcept
		{
			return 0 == (Fabs(1.f - m00) + Fabs(m01) + Fabs(m02) + Fabs(m10) + Fabs(1.f - m11) + Fabs(m12) + Fabs(m20) + Fabs(m21) + Fabs(1.f - m22) + Fabs(m30) + Fabs(m31) + Fabs(m32));
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
		constexpr inline int IsNormal(float threshold = 0.001) const noexcept
		{
			int a = (Fabs(1 - (GetColumn3(0) * GetColumn3(0)))) < threshold;
			int b = (Fabs(1 - (GetColumn3(1) * GetColumn3(1)))) < threshold;
			int c = (Fabs(1 - (GetColumn3(2) * GetColumn3(2)))) < threshold;
			return a & b & c;
		}
		bool operator==(const Matrix4x3& m) const
		{
			return memcmp(this, &m, sizeof(*this)) == 0;
		}
		bool operator!=(const Matrix4x3& m) const
		{
			return memcmp(this, &m, sizeof(*this)) != 0;
		}
	};
	std::ostream& operator<<(std::ostream& stream, const Matrix4x3& m);
	std::istream& operator>>(std::istream& stream, Matrix4x3& m);
}
