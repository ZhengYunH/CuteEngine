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
	void PostProcessRenderPass::PrepareData()
	{
		IRenderPass::PrepareData();
		if (!mElement_)
		{
			mElement_ = new VulkanRenderElement(mPrim_, RenderSet::POSTPROCESS);
		}
		GEngine->Scene->GetRenderScene()->AddRenderElement(RenderSet::POSTPROCESS, mElement_);
	}

	PostProcessRenderPass::~PostProcessRenderPass()
	{
		SafeDestroy(mElement_);
		SafeDestroy(mPrim_);
		SafeDestroy(mMaterial);
	}

}

