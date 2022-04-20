#pragma once
#include "Common/Config.h"


namespace zyh
{
	class IRenderPass;
	class IRenderScene;
	class VulkanBase;

	class Renderer
	{
	public:
		Renderer(IRenderScene* renderScene);

		void Build();
		void Draw();

	protected:
		VulkanBase* mPlatform_;
		IRenderScene* mRenderScene_;
		std::vector<IRenderPass*> mRenderPasses_;

		size_t mCurrentImage_ = 0;
	};
}