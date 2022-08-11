#include "Renderer.h"
#include "IRenderPass.h"
#include "IRenderScene.h"

#include "Graphics/Vulkan/VulkanBase.h"
#include "Graphics/Vulkan/VulkanRenderElement.h"


#include "Graphics/Vulkan/VulkanLogicalDevice.h"
#include "Graphics/Vulkan/VulkanSurface.h"
#include "Graphics/Vulkan/VulkanSwapchain.h"


namespace zyh
{
	Renderer::Renderer(IRenderScene* renderScene)
		: mRenderScene_(renderScene)
	{
		mPlatform_ = new VulkanBase();
	}

	Renderer::~Renderer()
	{
		
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

		ImGuiRenderPass* uiPass = new ImGuiRenderPass(
			"GUI",
			{ RenderSet::SCENE },
			GVulkanInstance->mUIRenderPass_
		);
		mRenderPasses_.push_back(uiPass);
	}

	void Renderer::Draw()
	{
		RenderSet renderSet = RenderSet::SCENE;
		mPlatform_->DrawFrameBegin(mCurrentImage_);
		{
			for (auto& renderElement : mRenderScene_->GetRenderElements(renderSet))
			{
				VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
				element->updateUniformBuffer(mCurrentImage_);
			}

			for (IRenderPass* pass : mRenderPasses_)
			{
				pass->Prepare(GVulkanInstance->GetSwapchainFrameBuffer());
				pass->Draw(renderSet);
			}
		}
		mPlatform_->DrawFrameEnd();
	}

}