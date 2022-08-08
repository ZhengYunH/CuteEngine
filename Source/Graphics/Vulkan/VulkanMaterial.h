#pragma once
#include "Graphics/Common/IMaterial.h"
#include "VulkanHeader.h"
#include "VulkanTools.h"
#include "IVulkanObject.h"
#include "Graphics/Light/LightBase.h"
#include "Graphics/Vulkan/VulkanRenderPass.h"
#include "Graphics/Common/Geometry.h"
#include "Graphics/Common/RenderStage.h"
#include "Graphics/Common/IPrimitive.h"


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
		
		// Temp: Clean after VertexFactory Implementation
		VulkanMaterial(IPrimitive* prim, VulkanRenderPassBase::OpType opType = VulkanRenderPassBase::OpType::LOADCLEAR_AND_STORE);
		IPrimitive* mPrim_ { nullptr };

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
		virtual void getBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) 
		{
			HYBRID_CHECK(mPrim_);
			mPrim_->GetBindingDescriptions(descriptions);
		}
		
		virtual void getAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
		{ 
			HYBRID_CHECK(mPrim_);
			mPrim_->GetAttributeDescriptions(descriptions);
		}
		virtual void getPushConstantRange(std::vector<VkPushConstantRange>& pushConstantRanges) {}
		virtual void PushConstant(std::string semantic, void* data){}
		virtual void BindPushConstant(VkCommandBuffer vkCommandBuffer) {}

		virtual DepthStencilState GetDepthStencilState() { return mMaterial_->DepthStencil; }
		virtual RasterizationState GetRasterizationState() { return mMaterial_->Rasterization; }

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
		virtual void createDescriptorSetData();

		std::unordered_map<uint32_t, class VulkanTexture*> mTextureImages_;
		class VulkanRenderPassBase* mRenderPass_{ nullptr };

	protected:
		virtual void _createDescriptorSetData(EShaderType state, IShaderParser* parser);
	};


	class ImGuiMaterial : public VulkanMaterial
	{
		struct PushConstBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

	public:
		ImGuiMaterial(IMaterial* material) : VulkanMaterial(material, VulkanRenderPassBase::OpType::LOAD_AND_STORE) {}

	public:
		virtual void createDescriptorSetData() override;
		virtual void getBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) override;
		virtual void getAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions) override;
		virtual void getPushConstantRange(std::vector<VkPushConstantRange>& pushConstantRanges) override;
		virtual void PushConstant(std::string semantic, void* data) override;
		virtual void BindPushConstant(VkCommandBuffer vkCommandBuffer);

	};
}