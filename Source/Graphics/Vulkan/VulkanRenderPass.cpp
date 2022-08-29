#include "VulkanRenderPass.h"
#include "VulkanLogicalDevice.h"
#include "VulkanRenderElement.h"
#include "VulkanMaterial.h"

#include "Graphics/Common/IRenderPass.h"
#include "Core/TerrainComponent.h"

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
		const std::vector<RenderTarget>& RenderTargets = mRenderPass_->GetRenderTargets();
		size_t attachmentSize = RenderTargets.size();
		const RenderTarget& DepthStencil = mRenderPass_->GetDepthStencilTarget();
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
			const RenderTarget& target = RenderTargets[index];
			attachments[index].format = Convert::Format(target.Format);
			attachments[index].samples = Convert::Quality2SamplerCount(target.Quality);
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
			const RenderTarget& target = DepthStencil;
			attachments[index].format = Convert::Format(target.Format);
			attachments[index].samples = Convert::Quality2SamplerCount(target.Quality);
			attachments[index].stencilLoadOp = _convertLoadOp(target.LoadOp);
			attachments[index].stencilStoreOp = _convertStoreOp(target.StoreOp);

			attachments[index].initialLayout = target.LoadOp == RenderTarget::ELoadOp::LOAD ?
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

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
			attachments[index].format = Convert::Format(RenderTargets[0].Format);
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
			const std::vector<RenderTarget>& RenderTargets = mRenderPass_->GetRenderTargets();
			size_t attachmentSize = RenderTargets.size();
			const RenderTarget DepthStencil = mRenderPass_->GetDepthStencilTarget();
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
				element->setupState(this);
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

	struct UISettings {
		bool displayLogos = true;
		bool displayBackground = true;
		bool animateLight = false;
		float lightSpeed = 0.25f;
		std::array<float, 50> frameTimes{};
		float frameTimeMax = 9999.0f, frameTimeMin = 0.0f;
		float lightTimer = 0.0f;
	};

	class ImGuiRenderElement : public VulkanRenderElement
	{
	public:
		ImGuiRenderElement(VulkanMaterial* material) : VulkanRenderElement(material)
		{
		}

		virtual void setupState(class VulkanRenderPass* renderPass)
		{
			VulkanRenderElement::setupState(renderPass);
			
			uiSettings.frameTimes[GEngine->GetCurrFrame() % uiSettings.frameTimes.size()] = GEngine->GetDeltaTime() * 1000 * (uiSettings.frameTimeMax - uiSettings.frameTimeMin) / 10.f;

			ImGuiIO& io = ImGui::GetIO();
			float constantScale[2] = { 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y };
			float constantTranslate[2] = { -1.0f, -1.0f };

			mMaterial_->PushConstant("scale", &constantScale);
			mMaterial_->PushConstant("translate", &constantTranslate);

			NewFrame();
			UpdateBuffers();
		}

	protected:
		void NewFrame()
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui::NewFrame();
			// Init imGui windows and elements
			ImVec4 clear_color = ImColor(114, 144, 154);
			static float f = 0.0f;
			ImGui::TextUnformatted(Setting::AppTitle.c_str());

			ImGui::PlotLines("Frame Times", &uiSettings.frameTimes[0], 50, 0, "", uiSettings.frameTimeMin, uiSettings.frameTimeMax, ImVec2(0, 80));
			ImGui::Text("Camera");

			Matrix4x3 viewMatrix = GEngine->Scene->GetCamera()->getViewMatrix();
			float trans[3]{ viewMatrix.GetTranslation().x, viewMatrix.GetTranslation().y, viewMatrix.GetTranslation().z };
			float rot[3]{ viewMatrix.GetPitch(), viewMatrix.GetYaw(), viewMatrix.GetRoll() };
			float fov = GEngine->Scene->GetCamera()->getFov();

			ImGui::InputFloat3("position", trans);
			ImGui::InputFloat3("rotation", rot);
			ImGui::InputFloat("fov", &fov);
			ImGui::SetWindowPos(ImVec2(480, 350));
			ImGui::SetWindowSize(ImVec2(300, 230));

			ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);

			ImGui::Begin("Example settings");
			HeightMapManipulator* heightMapManipulator = HeightMapManipulator::getInstance();
			ImGui::Checkbox("Modify terrain", &heightMapManipulator->mEnable_);
			if (heightMapManipulator->mEnable_)
			{
				ImGui::SliderFloat("Modify Offset", &heightMapManipulator->modifyTerrainOffset, -10.f, 100.f);
				ImGui::SliderFloat("Modify Range", &heightMapManipulator->modifyTerrainRange, 1.f, 100.f);
			}
			ImGui::Checkbox("Display logos", &uiSettings.displayLogos);
			ImGui::Checkbox("Display background", &uiSettings.displayBackground);
			ImGui::Checkbox("Animate light", &uiSettings.animateLight);
			ImGui::SliderFloat("Light speed", &uiSettings.lightSpeed, 0.1f, 1.0f);
			ImGui::End();

			// Render to generate draw buffers
			ImGui::Render();
		}
	
		void UpdateBuffers()
		{
			ImDrawData* imDrawData = ImGui::GetDrawData();

			// Note: Alignment is done inside buffer creation
			VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
			VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

			if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
				return;
			}

			// Update buffers only if vertex or index count has been changed compared to current buffer size

			// Vertex buffer
			if (mVertexBuffer_ == nullptr || mVertexBuffer_->GetBufferSize() != vertexBufferSize)
			{
				SafeDestroy(mVertexBuffer_);
				mVertexBuffer_ = new VulkanBuffer();
				mVertexBuffer_->connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_);
				mVertexBuffer_->setup(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			}

			// Index buffer
			if (mIndexBuffer_ == nullptr || mIndexBuffer_->GetBufferSize() != indexBufferSize)
			{
				SafeDestroy(mIndexBuffer_);
				mIndexBuffer_ = new VulkanBuffer();
				mIndexBuffer_->connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_);
				mIndexBuffer_->setup(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			}

			VulkanBuffer stagingBuffer1, stagingBuffer2;
			stagingBuffer1.connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_);
			stagingBuffer1.setup(mVertexBuffer_->GetBufferSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			stagingBuffer2.connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_);
			stagingBuffer2.setup(mIndexBuffer_->GetBufferSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			size_t vtxOffset = 0;
			size_t idxOffset = 0;
			for (size_t i = 0; i < imDrawData->CmdListsCount; ++i)
			{
				const ImDrawList* cmdList = imDrawData->CmdLists[i];
				stagingBuffer1.setupData(cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert), vtxOffset);
				stagingBuffer2.setupData(cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx), idxOffset);
				vtxOffset += cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
				idxOffset += cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);
			}

			VulkanBuffer::copyBuffer(GVulkanInstance->mGraphicsCommandPool_, stagingBuffer1.Get().buffer, mVertexBuffer_->Get().buffer, mVertexBuffer_->GetBufferSize());
			VulkanBuffer::copyBuffer(GVulkanInstance->mGraphicsCommandPool_, stagingBuffer2.Get().buffer, mIndexBuffer_->Get().buffer, mIndexBuffer_->GetBufferSize());
		}

		VulkanBuffer* mVertexBuffer_{ nullptr };
		VulkanBuffer* mIndexBuffer_{ nullptr };
		UISettings uiSettings;
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

	void VulkanImGuiRenderPass::InitailizeResource()
	{
		GEngine->Scene->GetRenderScene()->AddRenderElement(RenderSet::UI, mRenderElement_);
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

