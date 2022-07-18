#pragma once
#include "Graphics/Common/IMaterial.h"
#include "VulkanHeader.h"
#include "VulkanTools.h"
#include "IVulkanObject.h"
#include "Graphics/Light/LightBase.h"


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

	const int N_MAX_POINT_LIGHT = 4;
	struct UniformLightingBufferObject
	{
		DirectionLight directionalLight;
		int32_t numOfPointLights;
		PointLight pointLights[N_MAX_POINT_LIGHT];
		SpotLight spotLight;
	};

	class VulkanMaterial : public IMaterial, public IVulkanObject
	{
	public:
		VulkanMaterial();

		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, uint32_t layoutCount);

		virtual void setup() override;

		virtual void cleanup() override;

	public:
		void beginUpdateUniformBuffer(size_t currentImage) { mCurrentUpdateImage_ = currentImage; }
		virtual void updateUniformBuffer(UniformBufferObject& ubo, UniformLightingBufferObject& ulbo);
		void endUpdateUniformBuffer(UniformBufferObject& ubo, UniformLightingBufferObject& ulbo);
		VkDescriptorSet getDescriptorSet(size_t currentImage) { return mDescriptorSets_[currentImage]; }
		VkPipelineLayout getPipelineLayout();

	protected:
		VulkanLogicalDevice* mLogicalDevice_;
		VulkanPhysicalDevice* mPhysicalDevice_;
		uint32_t			mLayoutCount_;
		size_t				mCurrentUpdateImage_;

	public:
		VulkanGraphicsPipeline* mGraphicsPipeline_;
		virtual void createGraphicsPipeline();

		std::vector<VulkanBuffer*> mUniformBuffers_;
		std::vector<VulkanBuffer*> mUniformLightBuffers_;
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