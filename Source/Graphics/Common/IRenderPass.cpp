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
#include "Graphics/Vulkan/VulkanRenderElement.h"

#include "Math/MathUtil.h"

#include "Core/TerrainComponent.h"
#include "Graphics/Common/IRenderScene.h"


namespace zyh
{
	class ImGuiRenderElement : public VulkanRenderElement
	{

	};

	ImGuiRenderPass::ImGuiRenderPass(const std::string& renderPassName, const TRenderSets& renderSets)
		: IRenderPass(renderPassName, renderSets)
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

	void ImGuiRenderPass::EmitRenderElements()
	{
		GEngine->Scene->GetRenderScene()->AddRenderElement(RenderSet::UI, mRenderElement_);
	}

	void ImGuiRenderPass::Init()
	{

	}

	void ImGuiRenderPass::PrepareData()
	{
		IRenderPass::PrepareData();
		uiSettings.frameTimes[GEngine->GetCurrFrame() % uiSettings.frameTimes.size()] = GEngine->GetDeltaTime() * 1000 * (uiSettings.frameTimeMax - uiSettings.frameTimeMin) / 10.f;

		ImGuiIO& io = ImGui::GetIO();
		float constantScale[2] = { 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y };
		float constantTranslate[2] = { -1.0f, -1.0f };
		mMaterial_->PushConstant("scale", &constantScale);
		mMaterial_->PushConstant("translate", &constantTranslate);

		NewFrame();
		UpdateBuffers();
		EmitRenderElements();
	}

	void PostProcessRenderPass::PrepareData()
	{
		IRenderPass::PrepareData();
		if (!mElement_)
		{
			mElement_ = new VulkanRenderElement(mPrim_, RenderSet::SCENE);
		}
	}

	PostProcessRenderPass::~PostProcessRenderPass()
	{
		SafeDestroy(mElement_);
		SafeDestroy(mPrim_);
		SafeDestroy(mMaterial);
	}

}

