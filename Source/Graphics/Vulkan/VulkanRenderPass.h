#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanLogicalDevice;

	class VulkanRenderPassBase : public TVulkanObject<VkRenderPass>
	{
	public:
		void connect(VulkanLogicalDevice* logicalDevice);
		virtual void setup(VkFormat colorFormat, VkSampleCountFlagBits msaaSample, VkFormat depthFormat);

	private:
		VulkanLogicalDevice* mVulkanLogicalDevice_;
	};
}