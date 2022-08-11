#pragma once
#include "Common/Config.h"
#include "Vector3.h"
#include "MathUtil.h"


namespace zyh
{
	class Box
	{
	public:
		Box() {}
		Box(const Vector3& BMin, const Vector3& BMax) : mBMin_(BMin), mBMax_(BMax){}

	public:
		const Vector3& GetMin() { return mBMin_; }
		const Vector3& GetMax() { return mBMax_; }

	protected:
		Vector3 mBMin_ { MIN_NUM };
		Vector3 mBMax_ { MAX_NUM };

	};
}