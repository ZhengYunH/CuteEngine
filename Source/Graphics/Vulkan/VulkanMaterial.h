#pragma once
#include "Graphics/Common/IMaterial.h"
#include "VulkanHeader.h"
#include "VulkanTools.h"
#include "IVulkanObject.h"


namespace zyh
{
	class VulkanLogicalDevice;
	class VulkanPhysicalDevice;
	class VulkanGraphicsPipeline;
	class VulkanBuffer;
	class VulkanTextureImage;

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	class VulkanMaterial : public IMaterial, public IVulkanObject
	{
	public:
		VulkanMaterial();

		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, uint32_t layoutCount);

		virtual void setup() override;

		virtual void cleanup() override;

	public:
		virtual void updateUniformBuffer(uint32_t currentImage);
		VkDescriptorSet getDescriptorSet() { return mDescriptorSets_[0]; }
		VkPipelineLayout getPipelineLayout() { return mGraphicsPipeline_->getPipelineLayout(); }

	protected:
		VulkanLogicalDevice* mLogicalDevice_;
		VulkanPhysicalDevice* mPhysicalDevice_;
		uint32_t			mLayoutCount_;

	protected:
		VulkanGraphicsPipeline* mGraphicsPipeline_;
		virtual void createGraphicsPipeline();

		std::vector<VulkanBuffer*> mUniformBuffers_;
		virtual void createUniformBuffers();

		VkDescriptorPool mDescriptorPool_;
		virtual void createDesciptorPool();

		std::vector<VkDescriptorSet> mDescriptorSets_;
		virtual void createDescriptorSets();

		VulkanTextureImage* mTextureImage_;

	};
}