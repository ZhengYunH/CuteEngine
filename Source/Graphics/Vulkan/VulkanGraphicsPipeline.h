#pragma once
#include "VulkanObject.h"

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

	public:
		const VkPipelineLayout& getPipelineLayout() { return mVkPipelineLayout_; }
		const VkDescriptorSetLayout& getDescriptorSetLayout() { return mVkDescriptorSetLayout_; }
	};
}