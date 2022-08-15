#pragma once
#include "VulkanObject.h"
#include "Graphics/Common/RenderStage.h"


namespace zyh
{
	class VulkanLogicalDevice;

	class VulkanGraphicsPipelineBase : public TVulkanObject<VkPipeline>
	{
	public:
		virtual void connect(VulkanLogicalDevice* logicalDevice);
		virtual void setup();

	protected:
		VulkanLogicalDevice* mVulkanLogicalDevice;

	protected:
		VkPipelineCache mVkPipelineCache_;
	};


	class VulkanGraphicsPipeline : public VulkanGraphicsPipelineBase
	{
	public:
		VulkanGraphicsPipeline(class VulkanMaterial* owner) : mOwner_(owner)
		{
		}

	public:
		virtual void connect(VulkanLogicalDevice* logicalDevice);
		virtual void setup();
		virtual void cleanup() override;

	protected:
		VkPipelineLayout mVkPipelineLayout_;
		virtual void _setupGraphicsPipeline();

	protected:
		VkCompareOp _convertCompareOp(ECompareOP op);
		VkStencilOp _convertStencilOp(DepthStencilState::EStencilOp op);
		VkBlendOp _convertBlendOp(ColorBlendState::EBlendOP op);
		VkBlendFactor _convertBlendFactor(ColorBlendState::EBlendFactor factor);

	public:
		const VkPipelineLayout& getPipelineLayout() { return mVkPipelineLayout_; }

	protected:
		class VulkanMaterial* mOwner_;
	};
}