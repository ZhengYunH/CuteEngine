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

#include "Graphics/Imgui/imgui.h"
#include "Graphics/Imgui/imgui_impl_vulkan.h"
#include "Graphics/Imgui/imgui_impl_win32.h"



namespace zyh
{
	class IRenderPass
	{
	public:
		IRenderPass(
			const std::string& renderPassName, 
			const TRenderSets& renderSets,
			VulkanRenderPassBase* renderPass
		):	mName_(renderPassName),
			mRenderSets_(renderSets),
			mRenderPass_(renderPass->Get())
		{
			InitRenderPass();
		}

		IRenderPass(
			const std::string& renderPassName,
			const TRenderSets& renderSets,
			VkRenderPass renderPass
		) : mName_(renderPassName),
			mRenderSets_(renderSets),
			mRenderPass_(renderPass)
		{
			InitRenderPass();
		}

		const bool IsRenderSetSupported(RenderSet renderSet)
		{
			return std::find(mRenderSets_.begin(), mRenderSets_.end(), renderSet) != mRenderSets_.end();
		}

		
		void Prepare(VkFramebuffer framebuffer);

		void Draw(RenderSet renderSet);
	
	protected:
		virtual void _DrawElements(VkCommandBuffer vkCommandBuffer, RenderSet renderSet);

	protected:
		void InitRenderPass();
		void InitCommandBufferBeginInfo();
		void InitRenderPassBeginInfo();
	
	protected:
		std::string mName_;
		TRenderSets mRenderSets_;
		VkRenderPass mRenderPass_;
		VkRenderPassBeginInfo mRenderPassInfo_{};
	
	private:
		VkFramebuffer mVKFramebuffer_;
		VkCommandBufferBeginInfo mVKBufferBeginInfo_{};
	};

	class ImGuiRenderPass : public IRenderPass
	{
	public:
		ImGuiRenderPass(
			const std::string& renderPassName,
			const TRenderSets& renderSets,
			VulkanRenderPassBase* renderPass
		) : IRenderPass(renderPassName, renderSets, renderPass)
		{
		}

		ImGuiRenderPass(
			const std::string& renderPassName,
			const TRenderSets& renderSets,
			VkRenderPass renderPass
		) : IRenderPass(renderPassName, renderSets, renderPass)
		{
		}

	public:
		ImDrawData* mDrawData_;

	protected:
		virtual void _DrawElements(VkCommandBuffer vkCommandBuffer, RenderSet renderSet) override;
	};
}