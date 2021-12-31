#include "MathUtil.h"

namespace zyh
{
	float WrapPI(float radians) noexcept
	{
		radians += MATH_PI;
		radians -= std::floor(radians * MATH_INV_DOUBLE_PI) * MATH_DOUBLE_PI;
		radians -= MATH_PI;
		return radians;
	}

	float SafeACos(float radians) noexcept
	{
		if (radians <= -1.f)
			return MATH_PI;
		if (radians >= 1.f)
			return 0.f;
		return ACos(radians);
	}
}