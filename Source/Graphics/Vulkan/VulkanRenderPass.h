#pragma once
#include "VulkanObject.h"
#include "Graphics/Common/RenderResource.h"
#include "Graphics/Common/IRenderPass.h"


namespace zyh
{
	class VulkanLogicalDevice;
	class VulkanImage;
	class VulkanFrameBuffer;

	class VulkanRenderTargetResource : public RenderTargetResource
	{
	public:
		VulkanRenderTargetResource(RenderTargetWrapper targetDesc) : RenderTargetResource(targetDesc)
		{
		}

	public:
		virtual void Create_Imp() override;

	public:
		virtual VkImageView GetImageView();

	protected:
		VulkanImage* mImage_;
	};

	class VulkanSwapchainResource : public VulkanRenderTargetResource
	{
	public:
		VulkanSwapchainResource(RenderTargetWrapper targetDesc) : VulkanRenderTargetResource(targetDesc)
		{
		}

	public:
		virtual void Create_Imp() override;

	public:
		virtual VkImageView GetImageView();
	};


	class VulkanRenderDevice : public RenderDevice
	{
	public:
		virtual RenderTargetResource* CreateRenderTarget_Imp(RenderTargetWrapper targetDesc) override
		{
			if (targetDesc.IsSwapChain)
				return new VulkanSwapchainResource(targetDesc);
			return new VulkanRenderTargetResource(targetDesc);
		}
	};

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

		virtual void Prepare();
		virtual void Draw();

	protected:
		virtual void _DrawElements(VkCommandBuffer vkCommandBuffer);

	protected:
		VulkanLogicalDevice* mVulkanLogicalDevice_;

	public:
		virtual void InitailizeResource();
		IRenderPass* GetRenderPass() { return mRenderPass_; }
		void CreateFrameBuffer();

	protected:
		IRenderPass* mRenderPass_;

	protected:
		std::vector<VulkanFrameBuffer*> mFrameBuffers_;
		VkCommandBufferBeginInfo mVKBufferBeginInfo_{};
		VkRenderPassBeginInfo mRenderPassInfo_{};
	};

	class VulkanImGuiRenderPass : public VulkanRenderPass
	{
	public:
		virtual void setup() override;
		virtual void InitailizeResource() override;

	protected:
		virtual void _DrawElements(VkCommandBuffer vkCommandBuffer) override;

	private:
		class VulkanMaterial* mMaterial_{ nullptr };
		class VulkanBuffer* mVertexBuffer_{ nullptr };
		class VulkanBuffer* mIndexBuffer_{ nullptr };
		class ImGuiRenderElement* mRenderElement_;
	};
}