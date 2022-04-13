#pragma once
#include "Common/Config.h"
#include "Graphics/Common/Geometry.h"
#include "Graphics/Common/ResourceLoader.h"
#include "Graphics/Common/IPrimitive.h"


namespace zyh
{
	class IRenderElement
	{
	public:
		IRenderElement(IPrimitive* InPrimtives)
			: mPrimitives_(InPrimtives)
		{
		}

	protected:
		IPrimitive* mPrimitives_;
	};
}
