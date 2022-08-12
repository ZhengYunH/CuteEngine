#pragma once
#include "VulkanObject.h"
#include "Graphics/Common/IRenderPass.h"


namespace zyh
{
	class VulkanLogicalDevice;
	class VulkanRenderPass : public TVulkanObject<VkRenderPass>
	{
	public:
		VulkanRenderPass(IRenderPass* renderPass)
			: mRenderPass_(renderPass)
		{

		}

		~VulkanRenderPass()
		{

		}

	public:
		void connect(VulkanLogicalDevice* logicalDevice);
		virtual void setup();
		virtual void cleanup() override;

		virtual void Prepare(VkFramebuffer framebuffer);
		virtual void Draw();

	protected:
		virtual void _DrawElements(VkCommandBuffer vkCommandBuffer);

	protected:
		VulkanLogicalDevice* mVulkanLogicalDevice_;

	public:
		void InitailizeResource()
		{
			connect(GVulkanInstance->mLogicalDevice_);
			setup();
		}

	protected:
		VkFormat _convertFormat(const EPixelFormat format);
		VkSampleCountFlagBits _convertQuality(const ESamplerQuality quality);
		VkAttachmentLoadOp _convertLoadOp(const RenderTarget::ELoadOp op);
		VkAttachmentStoreOp _convertStoreOp(const RenderTarget::EStoreOp op);

	protected:
		IRenderPass* mRenderPass_;

	protected:
		VkFramebuffer mVKFramebuffer_;
		VkCommandBufferBeginInfo mVKBufferBeginInfo_{};
		VkRenderPassBeginInfo mRenderPassInfo_{}
	};

}