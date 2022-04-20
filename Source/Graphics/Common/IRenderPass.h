#pragma once
// different from VulkanRenderPass
// IRenderPass: Define logical RenderPass, include:
//	- FrameBuffer
//	- Target RenderSet
//	- VulkanRenderBuffer
//	- ...
//	refer vkCmdBeginRenderPass / vkCmdEndRenderPass to understand this class
// VulkanRenderPass: Define Attachment(only metadata, 
//	any pipeline satisfies specific struct can use it) 

#include "Common/Config.h"
#include "Graphics/Vulkan/VulkanRenderPass.h"
#include "Graphics/Vulkan/VulkanCommandPool.h"

// usage



namespace zyh
{
	class IRenderPass
	{
	public:
		IRenderPass(
			const std::string& renderPassName, 
			const TRenderSets& renderSets,
			VulkanRenderPassBase* renderPass,
			VkFramebuffer framebuffer
		):	mName_(renderPassName),
			mRenderSets_(renderSets),
			mRenderPass_(renderPass),
			mVKFramebuffer_(framebuffer)
		{
			InitRenderPass();
		}

		const bool IsRenderSetSupported(RenderSet renderSet)
		{
			return std::find(mRenderSets_.begin(), mRenderSets_.end(), renderSet) != mRenderSets_.end();
		}

		void Draw(RenderSet renderSet);
	
	protected:
		void InitRenderPass();
		void InitCommandBufferBeginInfo();
		void InitRenderPassBeginInfo();
	
	protected:
		std::string mName_;
		TRenderSets mRenderSets_;
		VulkanRenderPassBase* mRenderPass_;
		VkRenderPassBeginInfo mRenderPassInfo_{};
	
	private:
		VkFramebuffer mVKFramebuffer_;
		VkCommandBufferBeginInfo mVKBufferBeginInfo_{};
	};
}