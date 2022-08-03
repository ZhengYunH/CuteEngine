#include "IRenderPass.h"
#include "Core/Engine.h"
#include "Core/ClientScene.h"
#include "Graphics/Vulkan/VulkanRenderElement.h"
#include "Graphics/Vulkan/VulkanBase.h"
#include "Graphics/Vulkan/VulkanInstance.h"


namespace zyh
{

	void IRenderPass::Prepare(VkFramebuffer framebuffer)
	{
		mVKFramebuffer_ = framebuffer;
	}

	void IRenderPass::Draw(RenderSet renderSet)
	{
		if (!IsRenderSetSupported(renderSet))
			return;

		mVKBufferBeginInfo_.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		mVKBufferBeginInfo_.flags = 0;
		mVKBufferBeginInfo_.pInheritanceInfo = nullptr;

		mRenderPassInfo_.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		mRenderPassInfo_.renderPass = mRenderPass_;
		mRenderPassInfo_.framebuffer = mVKFramebuffer_;
		mRenderPassInfo_.renderArea.offset = { 0, 0 };
		mRenderPassInfo_.renderArea.extent = *(GInstance->mExtend_);

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };
		mRenderPassInfo_.clearValueCount = static_cast<uint32_t>(clearValues.size());
		mRenderPassInfo_.pClearValues = clearValues.data();

		VulkanCommand* command = GVulkanInstance->GetCommandBuffer();

		command->begin(&mVKBufferBeginInfo_);
		{
			VkCommandBuffer vkCommandBuffer = command->Get();
			vkCmdBeginRenderPass(vkCommandBuffer, &mRenderPassInfo_, VK_SUBPASS_CONTENTS_INLINE);
			_DrawElements(vkCommandBuffer, renderSet);
			vkCmdEndRenderPass(vkCommandBuffer);
		}
		command->end();
	}

	void IRenderPass::_DrawElements(VkCommandBuffer vkCommandBuffer, RenderSet renderSet)
	{
		for (auto& renderElement : GEngine->Scene->GetRenderElements(renderSet))
		{
			VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
			element->draw(vkCommandBuffer, GVulkanInstance->GetCurrentImage());
		}
	}

	void IRenderPass::InitRenderPass()
	{
		InitCommandBufferBeginInfo();
		InitRenderPassBeginInfo();
	}

	void IRenderPass::InitCommandBufferBeginInfo()
	{

	}

	void IRenderPass::InitRenderPassBeginInfo()
	{

	}

	ImGuiRenderPass::ImGuiRenderPass(const std::string& renderPassName, const TRenderSets& renderSets, VulkanRenderPassBase* renderPass)
		: IRenderPass(renderPassName, renderSets, renderPass)
	{
		mMaterial_ = new VulkanMaterial(new IMaterial("Resource/shaders/ui.vert.spv", "Resource/shaders/ui.frag.spv"));
		mMaterial_->connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_, *GInstance->mImageCount_);;
		mMaterial_->setup();
	}

	ImGuiRenderPass::ImGuiRenderPass(const std::string& renderPassName, const TRenderSets& renderSets, VkRenderPass renderPass)
		: IRenderPass(renderPassName, renderSets, renderPass)
	{
		mMaterial_ = new VulkanMaterial(new IMaterial("Resource/shaders/ui.vert.spv", "Resource/shaders/ui.frag.spv"));
		mMaterial_->connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_, *GInstance->mImageCount_);;
		mMaterial_->setup();
	}

	void ImGuiRenderPass::InitResource()
	{

	}

	void ImGuiRenderPass::_DrawElements(VkCommandBuffer vkCommandBuffer, RenderSet renderSet)
	{
		ImGui_ImplVulkan_RenderDrawData(mDrawData_, vkCommandBuffer, mMaterial_->mGraphicsPipeline_->Get());
	}

}

