#include "VulkanRenderPass.h"
#include "VulkanLogicalDevice.h"
#include "VulkanRenderElement.h"
#include "VulkanMaterial.h"

#include "Graphics/Common/IRenderPass.h"

#include "Graphics/Imgui/imgui.h"
#include "Graphics/Imgui/imgui_impl_vulkan.h"
#include "Graphics/Imgui/imgui_impl_win32.h"


namespace zyh
{
	void VulkanRenderPass::connect(VulkanLogicalDevice* logicalDevice)
	{
		mVulkanLogicalDevice_ = logicalDevice;
	}

	void VulkanRenderPass::setup()
	{
		const std::vector<RenderTarget*>& RenderTargets = mRenderPass_->GetRenderTargets();
		size_t attachmentSize = RenderTargets.size();
		const RenderTarget* DepthStencil = mRenderPass_->GetDepthStencilTarget();
		if (DepthStencil)
			attachmentSize += 1;

		bool enableAA = true;
		if (enableAA)
			attachmentSize += 1; // for resolves

		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> references;
		attachments.resize(attachmentSize);
		references.resize(attachmentSize);

		size_t index = 0;
		// Color Attachment
		for (index = 0; index < RenderTargets.size(); ++index)
		{
			const RenderTarget& target = *RenderTargets[index];
			attachments[index].format = _convertFormat(target.Format);
			attachments[index].samples = _convertQuality(target.Quality);
			attachments[index].loadOp = _convertLoadOp(target.LoadOp);
			attachments[index].storeOp = _convertStoreOp(target.StoreOp);

			attachments[index].initialLayout = target.LoadOp == RenderTarget::ELoadOp::LOAD ?
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

			attachments[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[index].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			references[index].attachment = static_cast<uint32_t>(index);
			references[index].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		// Depth Attachment
		int depthStencilIndex = -1;
		if (DepthStencil)
		{
			depthStencilIndex = static_cast<int>(index);
			const RenderTarget& target = *RenderTargets[index];
			attachments[index].format = _convertFormat(target.Format);
			attachments[index].samples = _convertQuality(target.Quality);
			attachments[index].stencilLoadOp = _convertLoadOp(target.LoadOp);
			attachments[index].stencilStoreOp = _convertStoreOp(target.StoreOp);

			attachments[index].initialLayout = target.LoadOp == RenderTarget::ELoadOp::LOAD ?
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

			attachments[index].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[index].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[index].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			references[index].attachment = static_cast<uint32_t>(index);
			references[index].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			++index;
		}

		// Color Resolve Attachment
		int resolveIndex = -1;
		if (enableAA && !RenderTargets.empty())
		{
			resolveIndex = static_cast<int>(index);
			attachments[index].format = _convertFormat(RenderTargets[0]->Format);
			attachments[index].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[index].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[index].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[index].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[index].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			references[index].attachment = static_cast<uint32_t>(index);
			references[index].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			++index;
		}

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = static_cast<uint32_t>(RenderTargets.size());
		subpassDescription.pColorAttachments = references.data();
		subpassDescription.pDepthStencilAttachment = depthStencilIndex > 0 ? &references[depthStencilIndex] : VK_NULL_HANDLE;
		subpassDescription.pResolveAttachments = resolveIndex > 0 ? &references[resolveIndex] : VK_NULL_HANDLE;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_CHECK_RESULT(vkCreateRenderPass(mVulkanLogicalDevice_->Get(), &renderPassInfo, nullptr, &mVkImpl_));
	}

	void VulkanRenderPass::cleanup()
	{
		vkDestroyRenderPass(mVulkanLogicalDevice_->Get(), mVkImpl_, nullptr);
	}

	void VulkanRenderPass::Prepare(VkFramebuffer framebuffer)
	{
		mVKFramebuffer_ = framebuffer;
	}

	void VulkanRenderPass::Draw()
	{
		mVKBufferBeginInfo_.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		mVKBufferBeginInfo_.flags = 0;
		mVKBufferBeginInfo_.pInheritanceInfo = nullptr;

		mRenderPassInfo_.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		mRenderPassInfo_.renderPass = mVkImpl_;
		mRenderPassInfo_.framebuffer = mVKFramebuffer_;
		mRenderPassInfo_.renderArea.offset = { 0, 0 };
		mRenderPassInfo_.renderArea.extent = *(GInstance->mExtend_);
		{
			// set Clear Values
			const std::vector<RenderTarget*>& RenderTargets = mRenderPass_->GetRenderTargets();
			size_t attachmentSize = RenderTargets.size();
			const RenderTarget* DepthStencil = mRenderPass_->GetDepthStencilTarget();
			if (DepthStencil)
				attachmentSize += 1;
			std::vector<VkClearValue> clearValues;
			clearValues.resize(attachmentSize, {});
			size_t index = 0;
			for (index = 0; index < RenderTargets.size(); ++index)
			{
				clearValues[index].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			}
			clearValues[index].depthStencil = { 1.0f, 0 };
			++index;
			mRenderPassInfo_.clearValueCount = static_cast<uint32_t>(clearValues.size());
			mRenderPassInfo_.pClearValues = clearValues.data();
		}
		VulkanCommand* command = GVulkanInstance->GetCommandBuffer();
		command->begin(&mVKBufferBeginInfo_);
		{
			VkCommandBuffer vkCommandBuffer = command->Get();
			vkCmdBeginRenderPass(vkCommandBuffer, &mRenderPassInfo_, VK_SUBPASS_CONTENTS_INLINE);
			_DrawElements(vkCommandBuffer);
			vkCmdEndRenderPass(vkCommandBuffer);
		}
		command->end();
	}

	void VulkanRenderPass::_DrawElements(VkCommandBuffer vkCommandBuffer)
	{
		auto& renderSets = mRenderPass_->GetRenderSets();
		for (const RenderSet& renderSet : renderSets)
		{
			for (auto& renderElement : GEngine->Scene->GetRenderElements(renderSet))
			{
				VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
				element->draw(vkCommandBuffer, GVulkanInstance->GetCurrentImage());
			}
		}
	}

	void VulkanRenderPass::InitailizeResource()
	{
		connect(GVulkanInstance->mLogicalDevice_);
		setup();
	}

	// Helper Function
	VkFormat VulkanRenderPass::_convertFormat(const EPixelFormat format)
	{
		switch (format)
		{
		case EPixelFormat::UNDEFINED:
			return VK_FORMAT_UNDEFINED;
		case EPixelFormat::A32R32G32B32F:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case EPixelFormat::A16B16G16R16F:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case EPixelFormat::R8G8B8A8:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case EPixelFormat::D32_SFLOAT_S8_UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		default:
			Unimplement(0);
			break;
		}
		return VK_FORMAT_MAX_ENUM;
	}

	VkSampleCountFlagBits VulkanRenderPass::_convertQuality(const ESamplerQuality quality)
	{
		switch (quality)
		{
		case ESamplerQuality::None:
			return VK_SAMPLE_COUNT_1_BIT;
		case ESamplerQuality::Quality2X:
			return VK_SAMPLE_COUNT_2_BIT;
		case ESamplerQuality::Quality4X:
			return VK_SAMPLE_COUNT_4_BIT;
		case ESamplerQuality::Quality8X:
			return VK_SAMPLE_COUNT_8_BIT;
		default:
			Unimplement(0);
			break;
		}
		return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
	}

	VkAttachmentLoadOp VulkanRenderPass::_convertLoadOp(const RenderTarget::ELoadOp op)
	{
		switch (op)
		{
		case RenderTarget::ELoadOp::LOAD:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		case RenderTarget::ELoadOp::CLEAR:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		case RenderTarget::ELoadOp::DONT_CARE:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		default:
			Unimplement(0);
			break;
		}
		return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
	}

	VkAttachmentStoreOp VulkanRenderPass::_convertStoreOp(const RenderTarget::EStoreOp op)
	{
		switch (op)
		{
		case RenderTarget::EStoreOp::STORE:
			return VK_ATTACHMENT_STORE_OP_STORE;
		case RenderTarget::EStoreOp::DONT_CARE:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		default:
			Unimplement(0);
			break;
		}
		return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
	}

	class ImGuiRenderElement : public VulkanRenderElement
	{
	public:
		ImGuiRenderElement(VulkanMaterial* material) : VulkanRenderElement(material)
		{
		}
	};

	void VulkanImGuiRenderPass::setup()
	{
		VulkanRenderPass::setup();

		IMaterial* material = new IMaterial("Resource/shaders/ui.vert.spv", "Resource/shaders/ui.frag.spv");
		material->GetPipelineState().DepthStencil = DepthStencilState{ false, false };
		material->GetPipelineState().Rasterization = RasterizationState{ ERasterizationCullMode::NONE };
		
		mMaterial_ = new ImGuiMaterial(material);
		mMaterial_->connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_, *GInstance->mImageCount_);;
		mMaterial_->setup();

		mRenderElement_ = new ImGuiRenderElement(mMaterial_);

		ImGui::CreateContext();
		ImGui_ImplVulkan_InitInfo initInfo
		{
			GVulkanInstance->mInstance_->Get(), // VkInstance Instance;
			GVulkanInstance->mPhysicalDevice_->Get(), // VkPhysicalDevice PhysicalDevice;
			GVulkanInstance->mLogicalDevice_->Get(), //VkDevice Device;
			GVulkanInstance->mLogicalDevice_->mFamilyIndices_->getIndexByQueueFamily(GRAPHICS), //uint32_t QueueFamily;
			GVulkanInstance->mLogicalDevice_->graphicsQueue(), //VkQueue  Queue;
			VK_NULL_HANDLE, //VkPipelineCache PipelineCache;
			mMaterial_->mDescriptorPool_, //VkDescriptorPool DescriptorPool;
			0, //uint32_t Subpass;
			*GInstance->mImageCount_, // uint32_t MinImageCount;          // >= 2
			*GInstance->mImageCount_, //Setting:: uint32_t ImageCount;             // >= MinImageCount
			*GInstance->mMsaaSamples_, // VkSampleCountFlagBits MSAASamples;            // >= VK_SAMPLE_COUNT_1_BIT (0 -> default to VK_SAMPLE_COUNT_1_BIT)
			nullptr,
			// const VkAllocationCallbacks* Allocator;
			// void (*CheckVkResultFn)(VkResult err);
		};
		ImGui_ImplVulkan_Init(&initInfo, mVkImpl_);

		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)GVulkanInstance->GetScreenWidth(), (float)GVulkanInstance->GetScreenHeigth());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	}

	void VulkanImGuiRenderPass::_DrawElements(VkCommandBuffer vkCommandBuffer)
	{
		ImGuiIO& io = ImGui::GetIO();

		VkDescriptorSet descriptorSet = mMaterial_->getDescriptorSet(GVulkanInstance->GetCurrentImage());
		vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mMaterial_->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
		vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mMaterial_->getPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = ImGui::GetIO().DisplaySize.x;
		viewport.height = ImGui::GetIO().DisplaySize.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);

		mMaterial_->BindPushConstant(vkCommandBuffer);
		// Render commands
		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		if (imDrawData->CmdListsCount > 0) {

			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, &mVertexBuffer_->Get().buffer, offsets);
			vkCmdBindIndexBuffer(vkCommandBuffer, mIndexBuffer_->Get().buffer, 0, VK_INDEX_TYPE_UINT16);

			for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
			{
				const ImDrawList* cmd_list = imDrawData->CmdLists[i];
				for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
					VkRect2D scissorRect;
					scissorRect.offset.x = Max((int32_t)(pcmd->ClipRect.x), 0);
					scissorRect.offset.y = Max((int32_t)(pcmd->ClipRect.y), 0);
					scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
					scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
					vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissorRect);
					vkCmdDrawIndexed(vkCommandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
					indexOffset += pcmd->ElemCount;
				}
				vertexOffset += cmd_list->VtxBuffer.Size;
			}
		}
	}

}

