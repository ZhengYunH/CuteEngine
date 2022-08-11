#pragma once
#include "VulkanObject.h"
#include "Graphics/Common/RenderStage.h"


namespace zyh
{
	class VulkanLogicalDevice;
	class VulkanRenderPassBase;


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
		virtual void connect(VulkanLogicalDevice* logicalDevice, VulkanRenderPassBase* renderPass);
		virtual void setup();
		virtual void cleanup() override;

	protected:
		VulkanRenderPassBase* mVulkanRenderPass_;

	protected:
		VkPipelineLayout mVkPipelineLayout_;
		VkDescriptorSetLayout mVkDescriptorSetLayout_;
		virtual void _setupDescriptorSetLayout();
		virtual void _setupGraphicsPipeline();

	protected:
		VkCompareOp _convertCompareOp(ECompareOP op);
		VkStencilOp _convertStencilOp(DepthStencilState::EStencilOp op);

	public:
		const VkPipelineLayout& getPipelineLayout() { return mVkPipelineLayout_; }
		const VkDescriptorSetLayout& getDescriptorSetLayout() { return mVkDescriptorSetLayout_; }

	protected:
		class VulkanMaterial* mOwner_;
	};
}