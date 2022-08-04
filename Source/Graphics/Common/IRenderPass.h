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
#include "Common/KeyCodes.h"

#include "Graphics/Vulkan/VulkanHeader.h"
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

		
		virtual void Prepare(VkFramebuffer framebuffer);

		virtual void InitResource() {}

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

	struct UISettings {
		bool displayModels = true;
		bool displayLogos = true;
		bool displayBackground = true;
		bool animateLight = false;
		float lightSpeed = 0.25f;
		std::array<float, 50> frameTimes{};
		float frameTimeMax = 9999.0f, frameTimeMin = 0.0f;
		float lightTimer = 0.0f;
	};

	struct PushConstBlock {
		glm::vec2 scale;
		glm::vec2 translate;
	};

	class ImGuiRenderPass : public IRenderPass
	{
	private:
		PushConstBlock pushConstBlock;
		UISettings uiSettings;

	public:
		ImGuiRenderPass(
			const std::string& renderPassName,
			const TRenderSets& renderSets,
			VulkanRenderPassBase* renderPass
		);

		ImGuiRenderPass(
			const std::string& renderPassName,
			const TRenderSets& renderSets,
			VkRenderPass renderPass
		);

		virtual void Prepare(VkFramebuffer framebuffer) override;
		virtual void InitResource() override;

		~ImGuiRenderPass();

		void NewFrame();
		void UpdateBuffers();

	public:
		class VulkanMaterial* mMaterial_;
		class VulkanBuffer* mVertexBuffer_;
		class VulkanBuffer* mIndexBuffer_;
		bool mIsResourceDirty_{ true };

	protected:
		virtual void _DrawElements(VkCommandBuffer vkCommandBuffer, RenderSet renderSet) override;
		void InitInputBinding();

	protected: // Event Binding
		void EventLeftMouseDown(KEY_TYPE x, KEY_TYPE y);
		void EventRightMouseDown(KEY_TYPE x, KEY_TYPE y);
		void EventLeftMouseUp(KEY_TYPE x, KEY_TYPE y);
		void EventRightMouseUp(KEY_TYPE x, KEY_TYPE y);
		void EventMouseMove(KEY_TYPE x, KEY_TYPE y);

	private:
		void Init();
	};
}