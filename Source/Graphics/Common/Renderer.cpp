#include "Renderer.h"
#include "IRenderPass.h"
#include "IRenderScene.h"

#include "Graphics/Vulkan/VulkanBase.h"
#include "Graphics/Vulkan/VulkanRenderElement.h"
#include "Graphics/Vulkan/VulkanRenderPass.h"

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
				"Scene",
				{ RenderSet::SCENE },
				GVulkanInstance->mRenderPass_
			)
		);

		mRenderPasses_.push_back(
			new IRenderPass(
				"XRayWriter",
				{ RenderSet::XRAY },
				GVulkanInstance->mUIRenderPass_
			)
		);

		mRenderPasses_.push_back(
			new XRayPass(
				"XRayPostProcess",
				GVulkanInstance->mUIRenderPass_
			)
		);

		mRenderPasses_.push_back(
			new ImGuiRenderPass(
				"GUI",
				{ RenderSet::SCENE },
				GVulkanInstance->mUIRenderPass_
		));
	}

	void Renderer::Draw()
	{
		mPlatform_->DrawFrameBegin(mCurrentImage_);
		{
			for (auto renderSet : mRenderScene_->GetExistRenderSets())
			{
				for (auto& renderElement : mRenderScene_->GetRenderElements(renderSet))
				{
					VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
					element->updateUniformBuffer(mCurrentImage_);
				}
			}

			for (IRenderPass* pass : mRenderPasses_)
			{
				pass->Prepare(GVulkanInstance->GetSwapchainFrameBuffer());
				pass->Draw();
			}
		}
		mPlatform_->DrawFrameEnd();
	}

	void Renderer::Connect()
	{

	}

	void Renderer::Compile()
	{
		for (auto* renderPass : mVulkanRenderPasses_)
			SafeDestroy(renderPass);

		mVulkanRenderPasses_.clear();
		for (IRenderPass* pass : mRenderPasses_)
		{
			pass->PrepareData();
			VulkanRenderPass* vulkanPass = new VulkanRenderPass(pass);
			vulkanPass->InitailizeResource();
			mVulkanRenderPasses_.push_back(vulkanPass);
		}
	}

}