#pragma once
#include "Graphics/Common/IRenderElement.h"
#include "IVulkanObject.h"
#include "VulkanBuffer.h"
#include "VulkanMaterial.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanInstance.h"
#include "VulkanBase.h"
#include "Math/Matrix4x4.h"
#include "Core/Engine.h"
#include "Core/ClientScene.h"
#include "Camera/Camera.h"


namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanCommandPool;

	class VulkanRenderElement : public IRenderElement, public IVulkanObject
	{
	public:
		VulkanRenderElement(IPrimitive* InPrimtives) : IRenderElement(InPrimtives)
		{
			mMaterial_ = new VulkanMaterial(InPrimtives);
			connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_, GVulkanInstance->mGraphicsCommandPool_);
			setup();
		}

		virtual ~VulkanRenderElement() 
		{ 
			cleanup(); 
		}

	public:
		/// <summary>
		/// Implementation of IVulkanObject
		/// </summary>
		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool)
		{
			mMaterial_->connect(physicalDevice, logicalDevice, *GInstance->mImageCount_);;

			mVulkanPhysicalDevice_ = physicalDevice;
			mVulkanLogicalDevice_ = logicalDevice;
			mVulkanCommandPool_ = commandPool;
		}

		virtual void setup() override
		{
			mMaterial_->setup();

			// TODO: Global stagingBuffer
			VulkanBuffer stagingBuffer;
			stagingBuffer.connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);

			// create Vertex Buffer
			void* vertexData{ nullptr }; size_t vertexSize;
			void* indexData{ nullptr };	size_t indexSize;
			mPrimitives_->GetVerticesData(&vertexData, vertexSize);
			mPrimitives_->GetIndicesData(&indexData, indexSize);

			mVertexBuffer_ = new VulkanBuffer();
			mVertexBuffer_->connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);
			mVertexBuffer_->setup(vertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			stagingBuffer.setup(vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.setupData(vertexData, vertexSize);
			VulkanBuffer::copyBuffer(mVulkanCommandPool_, stagingBuffer.Get().buffer, mVertexBuffer_->Get().buffer, vertexSize);

			mIndexBuffer_ = new VulkanBuffer();
			mIndexBuffer_->connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);
			mIndexBuffer_->setup(indexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			stagingBuffer.setup(indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.setupData(indexData, indexSize);
			VulkanBuffer::copyBuffer(mVulkanCommandPool_, stagingBuffer.Get().buffer, mIndexBuffer_->Get().buffer, indexSize);
		}

		virtual void cleanup() override
		{
			SafeDestroy(mVertexBuffer_);
			SafeDestroy(mIndexBuffer_);
			SafeDestroy(mMaterial_);
		}

	public:
		void updateUniformBuffer(size_t currentImage)
		{
			UniformBufferObject ubo{};
			UniformLightingBufferObject ulbo{};

			auto convertToGlmMat = [](const Matrix4x4& mat) -> glm::mat4x4
			{
				return glm::mat4x4
				{
					mat.m00, mat.m01, mat.m02, mat.m03,
					mat.m10, mat.m11, mat.m12, mat.m13,
					mat.m20, mat.m21, mat.m22, mat.m23,
					mat.m30, mat.m31, mat.m32, mat.m33,
				};
			};
			
			Matrix4x3 modelMat = mPrimitives_->GetTransform();
			Matrix4x3 viewMat = GEngine->Scene->GetCamera()->getViewMatrix();
			Matrix4x4 projMat = GEngine->Scene->GetCamera()->getProjMatrix();

			ubo.model = convertToGlmMat(modelMat);
			ubo.view = convertToGlmMat(viewMat);
			ubo.proj = convertToGlmMat(projMat);
			ubo.proj[1][1] *= -1;

			ulbo.directionalLight = DirectionLight
			(
				Vector3(0.5f, 0.5f, 1.0f),
				Vector3(0.1f, 0.1f, 0.1f),
				Vector3(0.5f, 0.5f, 0.5f),
				Vector3(0.2f, 0.2f, 0.2f)
			);
			ulbo.numOfPointLights = 1;
			ulbo.pointLights[0] = PointLight
			(
				Vector3(-0.5f, -0.5f, 1.0f),
				Vector3(0.1f, 0.1f, 0.1f),
				Vector3(0.3f, 0.3f, 0.3f),
				Vector3(0.3f, 0.3f, 0.3f)
			);
			ulbo.spotLight = SpotLight
			(
				Vector3(0.5f, 0.5f, 1.0f),
				Vector3(-0.5f, -0.5f, -1.0f),
				Vector3(0.3f, 0.3f, 0.3f),
				Vector3(0.5f, 0.5f, 0.5f),
				Vector3(0.3f, 0.3f, 0.3f)
			);

			mMaterial_->beginUpdateUniformBuffer(currentImage);
			mMaterial_->updateUniformBuffer(ubo, ulbo);
			mMaterial_->endUpdateUniformBuffer(ubo, ulbo);
		}

		// TODO: should be optimized, less pipeline change
		void draw(VkCommandBuffer commandBuffer, size_t currImage)
		{
			size_t width = GVulkanInstance->GetScreenWidth();
			size_t height = GVulkanInstance->GetScreenHeigth();
			
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)GInstance->mExtend_->width;
			viewport.height = (float)GInstance->mExtend_->height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = *(GInstance->mExtend_);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mMaterial_->mGraphicsPipeline_->Get());
			VkDescriptorSet set = mMaterial_->getDescriptorSet(currImage);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mMaterial_->getPipelineLayout(), 0, 1, &set, 0, nullptr);

			uint32_t indexSize = mPrimitives_->GetIndicesCount();

			VkBuffer vertexBuffers[] = { mVertexBuffer_->Get().buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer_->Get().buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, indexSize, 1, 0, 0, 0);
		}

	protected:
		VulkanBuffer* mVertexBuffer_;
		VulkanBuffer* mIndexBuffer_;
	public:
		VulkanMaterial* mMaterial_;

	protected:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_;
		VulkanLogicalDevice* mVulkanLogicalDevice_;
		VulkanCommandPool* mVulkanCommandPool_;

	};
}