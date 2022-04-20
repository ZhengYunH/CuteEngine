#pragma once
#include "Common/Config.h"

namespace zyh
{
	class IMaterial
	{
	public:
		virtual ~IMaterial() {}
		virtual bool IsValid() const { return true; }
		virtual const TRenderSets& GetSupportRenderSet() const { return mRenderSets_; }

	public:
		TRenderSets mRenderSets_{ SCENE };
	};
}