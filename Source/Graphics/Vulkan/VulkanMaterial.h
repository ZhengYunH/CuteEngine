#pragma once
#include "Graphics/Common/IMaterial.h"
#include "VulkanHeader.h"
#include "VulkanTools.h"
#include "IVulkanObject.h"
#include "Graphics/Light/LightBase.h"
#include "Graphics/Vulkan/VulkanRenderPass.h"
#include "Graphics/Common/Geometry.h"

namespace zyh
{
	class VulkanLogicalDevice;
	class VulkanPhysicalDevice;
	class VulkanGraphicsPipeline;
	class VulkanUniformBuffer;
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

	class VulkanMaterial : public IVulkanObject
	{
		friend class VulkanGraphicsPipeline;
	public:
		VulkanMaterial(IMaterial* material, VulkanRenderPassBase::OpType opType = VulkanRenderPassBase::OpType::LOADCLEAR_AND_STORE);

		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, uint32_t layoutCount);

		virtual void setup() override;

		virtual void cleanup() override;

	public:
		void beginUpdateUniformBuffer(size_t currentImage) { mCurrentUpdateImage_ = currentImage; }
		virtual void updateUniformBuffer(UniformBufferObject& ubo, UniformLightingBufferObject& ulbo);
		void endUpdateUniformBuffer(UniformBufferObject& ubo, UniformLightingBufferObject& ulbo);
		VkDescriptorSet getDescriptorSet(size_t currentImage) { return mDescriptorSets_[currentImage]; }
		VkPipelineLayout getPipelineLayout();
		VkPipeline getPipeline();

		// TODO
		virtual void getBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) { descriptions.push_back(std::move(Vertex::getBindingDescription())); }
		virtual void getAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
		{ 
			auto descs = Vertex::getAttributeDescriptions();
			descriptions = std::vector<VkVertexInputAttributeDescription>(descs.begin(), descs.end());
		}
		virtual void getPushConstantRange(std::vector<VkPushConstantRange>& pushConstantRanges)
		{
		}
		virtual void getDepthTestInfo(bool& depthTestEnable, bool& depthWriteEnable, VkCompareOp& depthCompareOp)
		{
			depthTestEnable = true;
			depthWriteEnable = true;
			depthCompareOp = VK_COMPARE_OP_LESS;
		}

	protected:
		VulkanLogicalDevice* mLogicalDevice_;
		VulkanPhysicalDevice* mPhysicalDevice_;
		uint32_t			mLayoutCount_;
		size_t				mCurrentUpdateImage_;
		IMaterial*			mMaterial_;

	public:
		VulkanGraphicsPipeline* mGraphicsPipeline_;
		virtual void createGraphicsPipeline();


		VkDescriptorPool mDescriptorPool_;
		virtual void createDesciptorPool();

		std::vector<VkDescriptorSet> mDescriptorSets_;
		virtual void createDescriptorSets();

		std::unordered_map<uint32_t, std::vector<VulkanUniformBuffer*>> mUniformBuffers_;
		virtual void createUniformBuffers();

		std::unordered_map<uint32_t, class VulkanTexture*> mTextureImages_;
		virtual void createTextureImages();

		class VulkanRenderPassBase* mRenderPass_{ nullptr };
	};

	class ImGuiMaterial : public VulkanMaterial
	{
	public:
		ImGuiMaterial(IMaterial* material) : VulkanMaterial(material, VulkanRenderPassBase::OpType::LOAD_AND_STORE) {}

	public:
		virtual void createUniformBuffers() override {};
		virtual void createTextureImages() override;
		virtual void getBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) override;
		virtual void getAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions) override;
		virtual void getPushConstantRange(std::vector<VkPushConstantRange>& pushConstantRanges) override;
		virtual void getDepthTestInfo(bool& depthTestEnable, bool& depthWriteEnable, VkCompareOp& depthCompareOp)
		{
			depthTestEnable = false;
			depthWriteEnable = false;
			depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		}
	};
}