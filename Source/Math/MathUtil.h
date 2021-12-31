#pragma once
#include <cmath>
#include <tuple>

namespace zyh
{
	// Const
	static constexpr float MATH_PI = (3.141592653589793238462643383279502884197169399375105820974944592308f);
	static constexpr float MATH_INV_PI = (1.f / MATH_PI);
	static constexpr float MATH_DOUBLE_PI = (2 * MATH_PI);
	static constexpr float MATH_INV_DOUBLE_PI = (1.f / MATH_DOUBLE_PI);
	static constexpr float MATH_HALF_PI = (MATH_PI / 2.f);

	static constexpr float MATH_PI_OVER_180 = (MATH_PI / 180.0f);
	static constexpr float MATH_PI_UNDER_180 = (180.0f / MATH_PI);

	static constexpr float EPISILON = 1e-5f;

	static float WrapPI(float radians) noexcept; // clamp theta to (-pi, pi]
	static float SafeACos(float radians) noexcept;
	static inline float ACos(float radians) noexcept { return std::acos(radians); }
	static inline float Cos(float radians) noexcept { return std::cos(radians);  }
	static inline float Sin(float radians) noexcept { return std::sin(radians); }

	template<typename _Type>
	static constexpr inline _Type Sqrt(_Type value) noexcept { return std::sqrt(value); }

	template<typename _lhs, typename _rhs>
	static constexpr inline auto ATan2(_lhs a, _rhs b) noexcept
	{
		return std::atan2(a, b);
	}
	
	static inline std::tuple<float, float> SinCos(float radians) noexcept {
		return std::make_tuple(std::sin(radians), std::cos(radians));
	}

	typedef unsigned int uint32;
	static constexpr uint32 FLOATU32SignMask = (1UL << 31);

	static constexpr inline uint32 FLOATU32(float x) noexcept
	{
		union
		{
			float  f;
			uint32 i = 0;
		} s;
		s.f = x;
		return s.i;
	}
	static constexpr inline float FLOATFROMU32(uint32 x) noexcept
	{
		union
		{
			float  f;
			uint32 i = 0;
		} s;
		s.i = x;
		return s.f;
	}

	static constexpr inline float Sign(float x) noexcept {
		union
		{
			float f;
			uint32 i = 0;
		} u;
		u.f = x;
		u.i = u.i & (0x80000000UL | 0x3f800000UL);
		return u.f;
	}

	static constexpr inline float Fabs(float x) noexcept {
		unsigned int i = FLOATU32(x) & (~FLOATU32SignMask);
		return FLOATFROMU32(i);
	}

	template<typename _lhs, typename _rhs> 
	static constexpr inline auto Max(_lhs a, _rhs b) noexcept
	{
		return (a > b) ? (a) : (b);
	}

	template<typename _lhs, typename _rhs, typename... _type> 
	static constexpr inline auto Max(_lhs a, _rhs b, _type... c) noexcept
	{
		return Max(a, Max(b, c...));
	}

	template<typename _lhs, typename _rhs> 
	static constexpr inline auto Min(_lhs a, _rhs b) noexcept
	{
		return (a < b) ? (a) : (b);
	}
	template<typename _lhs, typename _rhs, typename... _type> 
	static constexpr inline auto Min(_lhs a, _rhs b, _type... c) noexcept
	{
		return Min(a, Min(b, c...));
	}

	template<typename _type> 
	static constexpr inline _type Clamp(_type value, _type minimum, _type maximum) noexcept
	{
		if (value < minimum)
			return minimum;
		if (value > maximum)
			return maximum;
		return value;
	}

	template<typename _type> 
	static constexpr inline std::enable_if_t<std::is_floating_point_v<_type>, _type> DegreeToRadian(const _type& angle) noexcept
	{
		return angle * MATH_PI_OVER_180;
	}
	template<typename _type> 
	static constexpr inline std::enable_if_t<std::is_floating_point_v<_type>, _type> RadianToDegree(const _type& radian) noexcept
	{
		return radian * MATH_PI_UNDER_180;
	}
	template<typename _type> 
	static constexpr inline std::enable_if_t<std::is_floating_point_v<_type>, _type> Round(const _type& a) noexcept
	{
		auto i = static_cast<int>(a);
		if ((a - _type(i)) < _type(0.5))
			return _type(i);
		return _type(i + 1);
	}

	template<typename _type, typename _scalar> 
	static constexpr inline _type Lerp(const _type& a, const _type& b, const _scalar& x) noexcept
	{
		return (a * (1 - x)) + (b * x);
	}

	template<typename _type, typename _scalar> 
	static constexpr inline _type CubicInterp(const _type& a, const _type& ta, const _type& b, const _type& tb, const _scalar& x) noexcept
	{
		const _scalar x2 = x * x;
		const _scalar x3 = x * x2;
		_scalar		  tmp = 2 * x3 - 3 * x2;
		return (tmp + 1) * a + (x3 - 2 * x2 + x) * ta + (-tmp) * b + (x3 - x2) * tb;
	}

	static inline float Fmod(float X, float Y)
	{
		if (Fabs(Y) <= 1.e-8f)
		{
			//error
			return 0.f;
		}
		const float Quotient = static_cast<float>(static_cast<int>(X / Y));
		float		IntPortion = Y * Quotient;

		// Rounding and imprecision could cause IntPortion to exceed X and cause the result to be outside the expected range.
		// For example Fmod(55.8, 9.3) would result in a very small negative value!
		if (Fabs(IntPortion) > Fabs(X))
		{
			IntPortion = X;
		}

		const float Result = X - IntPortion;
		return Result;
	}

	template<typename _type>
	static inline constexpr std::enable_if_t<std::is_floating_point_v<_type>, bool> IsZero(_type v)
	{
		return v < EPISILON && v > -EPISILON;
	}
}