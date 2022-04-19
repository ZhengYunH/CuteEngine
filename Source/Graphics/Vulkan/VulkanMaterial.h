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
		virtual void updateUniformBuffer(size_t currentImage);
		VkDescriptorSet getDescriptorSet(size_t currentImage) { return mDescriptorSets_[currentImage]; }
		VkPipelineLayout getPipelineLayout();

	protected:
		VulkanLogicalDevice* mLogicalDevice_;
		VulkanPhysicalDevice* mPhysicalDevice_;
		uint32_t			mLayoutCount_;

	public:
		VulkanGraphicsPipeline* mGraphicsPipeline_;
		virtual void createGraphicsPipeline();

		std::vector<VulkanBuffer*> mUniformBuffers_;
		virtual void createUniformBuffers();

		VkDescriptorPool mDescriptorPool_;
		virtual void createDesciptorPool();

		std::vector<VkDescriptorSet> mDescriptorSets_;
		virtual void createDescriptorSets();

		VulkanTextureImage* mTextureImage_;
		void createTextureImage();

		class VulkanRenderPassBase* mRenderPass_{ nullptr };
	};
}