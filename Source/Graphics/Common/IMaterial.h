#pragma once
#include "Common/Config.h"

namespace zyh
{
	typedef std::vector<RenderSet> TRenderSets;

	class IMaterial
	{
	public:
		virtual ~IMaterial() {}
		virtual bool IsValid() { return true; }
		virtual const TRenderSets& GetSupportRenderSet() { return mRenderSets_; }

	public:
		TRenderSets mRenderSets_;
	};
}