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
		virtual void setup(
			const std::string& vertShaderFile, const std::string& fragShaderFile,
			VkExtent2D extend, VkSampleCountFlagBits msaaSamples
		);
		virtual void prepare(
			const std::string& vertShaderFile, const std::string& fragShaderFile,
			VkExtent2D extend, VkSampleCountFlagBits msaaSamples
		);

	protected:
		VulkanRenderPassBase* mVulkanRenderPass_;

	protected:
		virtual void _setupGraphicsPipeline(
			const std::string& vertShaderFile, const std::string& fragShaderFile,
			VkExtent2D extend, VkSampleCountFlagBits msaaSamples
		);
		virtual void _setupDescriptorSetLayout();
		

	protected:
		VkPipelineLayout mVkPipelineLayout_;
		VkDescriptorSetLayout mVkDescriptorSetLayout_;
	};
}