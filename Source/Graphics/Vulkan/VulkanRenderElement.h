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
		VulkanRenderElement(IPrimitive* InPrimtives, RenderSet renderSet) : IRenderElement()
		{
			mMaterial_ = new VulkanMaterial(InPrimtives, renderSet);
			connect(GVulkanInstance->mPhysicalDevice_, GVulkanInstance->mLogicalDevice_, GVulkanInstance->mGraphicsCommandPool_);
			setup();
			updateData(InPrimtives);

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

			size_t maxBufferSize = *GInstance->mImageCount_;
			mVertexBuffers_.resize(maxBufferSize, nullptr);
			mIndexBuffers_.resize(maxBufferSize, nullptr);
		}

		virtual void cleanup() override
		{
			for (auto buffer : mVertexBuffers_)
				SafeDestroy(buffer);
			mVertexBuffers_.clear();
			for (auto buffer : mIndexBuffers_)
				SafeDestroy(buffer);
			mIndexBuffers_.clear();
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

		virtual void draw(VkCommandBuffer commandBuffer, size_t currImage)
		{
			HYBRID_CHECK(GetActiveVertexBuffer());
			HYBRID_CHECK(GetActiveIndexBuffer());

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
			if (mMaterial_->needUpdateDesciptorSet())
			{
				VkDescriptorSet set = mMaterial_->getDescriptorSet(currImage);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mMaterial_->getPipelineLayout(), 0, 1, &set, 0, nullptr);
			}

			uint32_t indexSize = GetActiveIndexBuffer()->GetBufferSize() / sizeof(uint32_t);
			VkBuffer vertexBuffers[] = { GetActiveVertexBuffer()->Get().buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, GetActiveIndexBuffer()->Get().buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, indexSize, 1, 0, 0, 0);
		}

		void updateData(
			void* vertexData, size_t vertexSize,
			void* indexData, size_t indexSize
		)
		{
			VulkanBuffer stagingBuffer;
			stagingBuffer.connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);

			uint32_t maxBufferSize = *GInstance->mImageCount_;
			bool needCreateInitBuffer = mActiveVertexBufferIndex_ < 0;
			mActiveVertexBufferIndex_ = (mActiveVertexBufferIndex_ + 1) % maxBufferSize;
			bool needRecreateVertexBuffer = needCreateInitBuffer || !GetActiveVertexBuffer() || GetActiveVertexBuffer()->GetBufferSize() != vertexSize;

			if (needRecreateVertexBuffer || !GetActiveVertexBuffer())
			{
				SafeDestroy(mVertexBuffers_[mActiveVertexBufferIndex_]);
				mVertexBuffers_[mActiveVertexBufferIndex_] = new VulkanBuffer();
				GetActiveVertexBuffer()->connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);
				GetActiveVertexBuffer()->setup(vertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			}

			stagingBuffer.setup(vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.setupData(vertexData, vertexSize);
			VulkanBuffer::copyBuffer(mVulkanCommandPool_, stagingBuffer.Get().buffer, GetActiveVertexBuffer()->Get().buffer, vertexSize);

			needCreateInitBuffer = mActiveIndexBufferIndex_ < 0;
			mActiveIndexBufferIndex_ = (mActiveIndexBufferIndex_ + 1) % maxBufferSize;
			bool needRecreateIndexBuffer = needCreateInitBuffer || !GetActiveIndexBuffer() || GetActiveIndexBuffer()->GetBufferSize() != indexSize;
			if (needRecreateIndexBuffer)
			{
				SafeDestroy(mIndexBuffers_[mActiveIndexBufferIndex_]);
				mIndexBuffers_[mActiveIndexBufferIndex_] = new VulkanBuffer();
				GetActiveIndexBuffer()->connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);
				GetActiveIndexBuffer()->setup(indexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			}
			stagingBuffer.setup(indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.setupData(indexData, indexSize);
			VulkanBuffer::copyBuffer(mVulkanCommandPool_, stagingBuffer.Get().buffer, GetActiveIndexBuffer()->Get().buffer, indexSize);
		}

		void updateData(IPrimitive* InPrimtives)
		{
			if (!InPrimtives)
				return;
			void* vertexData{ nullptr }; size_t vertexSize;
			void* indexData{ nullptr };	size_t indexSize;
			InPrimtives->GetVerticesData(&vertexData, vertexSize);
			InPrimtives->GetIndicesData(&indexData, indexSize);
			updateData(vertexData, vertexSize, indexData, indexSize);
		}

	protected:
		std::vector<VulkanBuffer*> mVertexBuffers_;
		std::vector<VulkanBuffer*> mIndexBuffers_;

		int mActiveVertexBufferIndex_{ -1 };
		int mActiveIndexBufferIndex_{ -1 };
			
		VulkanBuffer* GetActiveVertexBuffer()
		{
			HYBRID_CHECK(mActiveVertexBufferIndex_ < mVertexBuffers_.size());
			return mVertexBuffers_[mActiveVertexBufferIndex_];
		}

		VulkanBuffer* GetActiveIndexBuffer()
		{
			HYBRID_CHECK(mActiveIndexBufferIndex_ < mIndexBuffers_.size());
			return mIndexBuffers_[mActiveIndexBufferIndex_];
		}


	public:
		VulkanMaterial* mMaterial_{ nullptr };

	protected:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_{ nullptr };
		VulkanLogicalDevice* mVulkanLogicalDevice_{ nullptr };
		VulkanCommandPool* mVulkanCommandPool_{ nullptr };

	};
}