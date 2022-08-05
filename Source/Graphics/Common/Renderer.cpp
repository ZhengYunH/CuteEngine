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
		ImGui_ImplVulkanH_DestroyWindow(
			GVulkanInstance->mInstance_->Get(), 
			GVulkanInstance->mLogicalDevice_->Get(),
			&mGuiData_,
			nullptr /* Allocator */
		);
	}

	void Renderer::Build()
	{
		mPlatform_->Initialize();

		// GUI
		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		mGuiData_.Surface = GVulkanInstance->mSurface_->Get();
		mGuiData_.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
			GVulkanInstance->mPhysicalDevice_->Get(),
			mGuiData_.Surface,
			requestSurfaceImageFormat,
			(size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
			requestSurfaceColorSpace
		);

#ifdef IMGUI_UNLIMITED_FRAME_RATE
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
		mGuiData_.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
			GVulkanInstance->mPhysicalDevice_->Get(),
			mGuiData_.Surface,
			&present_modes[0],
			IM_ARRAYSIZE(present_modes)
		);

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