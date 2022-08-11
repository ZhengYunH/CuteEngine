#include "IRenderPass.h"
#include "Core/Engine.h"
#include "Core/ClientScene.h"
#include "Graphics/Vulkan/VulkanRenderElement.h"
#include "Graphics/Vulkan/VulkanBase.h"
#include "Graphics/Vulkan/VulkanInstance.h"
#include "Graphics/Vulkan/VulkanSurface.h"
#include "Graphics/Vulkan/VulkanPhysicalDevice.h"
#include "Graphics/Vulkan/VulkanLogicalDevice.h"
#include "Graphics/Vulkan/VulkanSwapchain.h"
#include "Graphics/Vulkan/VulkanCommandPool.h"
#include "Graphics/Vulkan/VulkanImage.h"
#include "Graphics/Vulkan/VulkanRenderPass.h"
#include "Graphics/Vulkan/VulkanGraphicsPipeline.h"
#include "Graphics/Vulkan/VulkanBuffer.h"

#include "Math/MathUtil.h"

#include "Core/TerrainComponent.h"


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
		Init();
	}

	ImGuiRenderPass::ImGuiRenderPass(const std::string& renderPassName, const TRenderSets& renderSets, VkRenderPass renderPass)
		: IRenderPass(renderPassName, renderSets, renderPass)
	{
		Init();
	}

	ImGuiRenderPass::~ImGuiRenderPass()
	{
		ImGui::DestroyContext();
		delete mMaterial_;
	}

	void ImGuiRenderPass::NewFrame()
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
		float trans[3]{ viewMatrix.GetTranslation().x, viewMatrix.GetTranslation().y, viewMatrix.GetTranslation().z};
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

	void ImGuiRenderPass::UpdateBuffers()
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

	void ImGuiRenderPass::Prepare(VkFramebuffer framebuffer)
	{
		IRenderPass::Prepare(framebuffer);
		NewFrame();
		UpdateBuffers();
		uiSettings.frameTimes[GEngine->GetCurrFrame() % uiSettings.frameTimes.size()] = GEngine->GetDeltaTime() * 1000 * (uiSettings.frameTimeMax - uiSettings.frameTimeMin) / 10.f;
	}

	void ImGuiRenderPass::InitResource()
	{
	}

	void ImGuiRenderPass::_DrawElements(VkCommandBuffer vkCommandBuffer, RenderSet renderSet)
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

		// UI scale and translate via push constants
		float constantScale[2] = { 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y };
		float constantTranslate[2] = { -1.0f, -1.0f };
		mMaterial_->PushConstant("scale", &constantScale);
		mMaterial_->PushConstant("translate", &constantTranslate);
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

	void ImGuiRenderPass::Init()
	{
		IMaterial* material = new IMaterial("Resource/shaders/ui.vert.spv", "Resource/shaders/ui.frag.spv");
		material->GetPipelineState().DepthStencil = DepthStencilState{false, false};
		material->GetPipelineState().Rasterization = RasterizationState{ ERasterizationCullMode::NONE };

		mMaterial_ = new ImGuiMaterial(material);
		mMaterial_->connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_, *GInstance->mImageCount_);;
		mMaterial_->setup();

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
		ImGui_ImplVulkan_Init(&initInfo, mRenderPass_);

		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)GVulkanInstance->GetScreenWidth(), (float)GVulkanInstance->GetScreenHeigth());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	}

}

