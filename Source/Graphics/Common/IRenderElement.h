#pragma once
#include "Common/Config.h"
#include "Math/Matrix4x4.h"


namespace zyh
{
	class IRenderElement
	{
	public:
		IRenderElement()
		{
			mTransform_.SetIdentity();
		}

		void UpdateTransform(Matrix4x3& transform)
		{
			mTransform_ = transform;
		}

	protected:
		Matrix4x3 mTransform_;
	};
}
