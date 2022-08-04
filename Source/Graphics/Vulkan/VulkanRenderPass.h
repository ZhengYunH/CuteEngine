#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanLogicalDevice;
	class VulkanRenderElement;

	class VulkanRenderPassBase : public TVulkanObject<VkRenderPass>
	{
	public:
		enum class OpType
		{
			LOADCLEAR_AND_STORE,
			LOAD_AND_STORE,
		};

	public:
		VulkanRenderPassBase(OpType opType)
			: mOpType_(opType)
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

	protected:
		VulkanLogicalDevice* mVulkanLogicalDevice_;
		OpType mOpType_;

		std::vector<VulkanRenderElement*> mElements_;
	};
}