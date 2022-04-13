#pragma once
#include "Common/Config.h"


namespace zyh
{
	class IRenderElement;

	class IRenderScene
	{
	public: 
		IRenderScene() {}

	public:
		bool AddRenderElement(RenderSet renderSet, IRenderElement* element) 
		{ 
			if (mRenderElements_.find(renderSet) == mRenderElements_.end())
			{
				mRenderElements_[renderSet] = std::vector<IRenderElement*>();
			}
			mRenderElements_[renderSet].push_back(element);
			mIsDirtys_[renderSet] = true;
			return true;
		}

		const std::vector<IRenderElement*>& GetRenderElements(RenderSet renderSet)
		{
			HYBRID_CHECK(mRenderElements_.find(renderSet) != mRenderElements_.end());

			return mRenderElements_[renderSet];
		}

		bool IsRenderElementDirty(RenderSet renderSet)
		{
			HYBRID_CHECK(mRenderElements_.find(renderSet) != mRenderElements_.end());
			
			return mIsDirtys_[renderSet];
		}

		void SetRenderElementDirty(RenderSet renderSet, bool IsDirty)
		{
			HYBRID_CHECK(mRenderElements_.find(renderSet) != mRenderElements_.end());

			mIsDirtys_[renderSet] = IsDirty;
		}

	private:
		std::map<RenderSet, std::vector<IRenderElement*>> mRenderElements_;
		std::map<RenderSet, bool> mIsDirtys_;
	};
}