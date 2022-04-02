#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanLogicalDevice;
	class VulkanRenderElement;

	class VulkanRenderPassBase : public TVulkanObject<VkRenderPass>
	{
	public:
		VulkanRenderPassBase(const std::string& vertShaderFile, const std::string& fragShaderFile) 
			: mVertShaderFile_(vertShaderFile), mFragShaderFile_(fragShaderFile)
		{}

		void connect(VulkanLogicalDevice* logicalDevice);
		virtual void setup(VkFormat colorFormat, VkSampleCountFlagBits msaaSample, VkFormat depthFormat);
		virtual void cleanup() override;

	public:
		virtual void AddElements(VulkanRenderElement* element)
		{
			mElements_.push_back(element);
		}

		virtual void Draw(VkCommandBuffer commandBuffer);

	public:
		const std::string& getVertShaderFile() { return mVertShaderFile_; }
		const std::string& getFragShaderFile() { return mFragShaderFile_; }

	protected:
		VulkanLogicalDevice* mVulkanLogicalDevice_;
		std::string mVertShaderFile_;
		std::string mFragShaderFile_;

		std::vector<VulkanRenderElement*> mElements_;
	};
}