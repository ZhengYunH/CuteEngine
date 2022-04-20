#include "Renderer.h"
#include "IRenderPass.h"
#include "IRenderScene.h"

#include "Graphics/Vulkan/VulkanBase.h"
#include "Graphics/Vulkan/VulkanRenderElement.h"


namespace zyh
{
	Renderer::Renderer(IRenderScene* renderScene)
		: mRenderScene_(renderScene)
	{
		mPlatform_ = new VulkanBase();
	}

	void Renderer::Build()
	{
		mPlatform_->Initialize();
		mRenderPasses_.push_back(
			new IRenderPass(
				"Test",
				{ RenderSet::SCENE },
				GVulkanInstance->mRenderPass_
			)
		);
	}

	void Renderer::Draw()
	{
		RenderSet renderSet = RenderSet::SCENE;

		mPlatform_->DrawFrameBegin(mCurrentImage_);

		for (auto& renderElement : mRenderScene_->GetRenderElements(renderSet))
		{
			VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
			element->mMaterial_->updateUniformBuffer(mCurrentImage_);
		}

		for (IRenderPass* pass : mRenderPasses_)
		{
			pass->Prepare(GVulkanInstance->GetSwapchainFrameBuffer());
			pass->Draw(renderSet);
		}

		mPlatform_->DrawFrameEnd();
	}

}