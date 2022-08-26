#pragma once
#include "VulkanObject.h"
#include "Graphics/Common/RenderResource.h"


namespace zyh
{
	class VulkanLogicalDevice;
	class IRenderPass;
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
		void InitailizeResource();
		IRenderPass* GetRenderPass() { return mRenderPass_; }

	protected:
		VkAttachmentLoadOp _convertLoadOp(const RenderTarget::ELoadOp op);
		VkAttachmentStoreOp _convertStoreOp(const RenderTarget::EStoreOp op);

	protected:
		IRenderPass* mRenderPass_;

	protected:
		VkFramebuffer mVKFramebuffer_;
		VkCommandBufferBeginInfo mVKBufferBeginInfo_{};
		VkRenderPassBeginInfo mRenderPassInfo_{};
	};

	class VulkanImGuiRenderPass : public VulkanRenderPass
	{
	public:
		virtual void setup() override;
	protected:
		virtual void _DrawElements(VkCommandBuffer vkCommandBuffer) override;

	private:
		class VulkanMaterial* mMaterial_{ nullptr };
		class VulkanBuffer* mVertexBuffer_{ nullptr };
		class VulkanBuffer* mIndexBuffer_{ nullptr };
		class ImGuiRenderElement* mRenderElement_;
	};
}