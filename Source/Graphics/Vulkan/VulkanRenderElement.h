#pragma once
#include "Graphics/Common/IRenderElement.h"
#include "IVulkanObject.h"
#include "VulkanBuffer.h"
#include "VulkanMaterial.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanInstance.h"


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
			mMaterial_ = new VulkanMaterial();
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

			mVertexBuffer_ = new VulkanBuffer();
			// create Vertex Buffer
			VkDeviceSize bufferSize = sizeof(mPrimitives_->mVertices_[0]) * mPrimitives_->mVertices_.size();
			mVertexBuffer_ = new VulkanBuffer();
			mVertexBuffer_->connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);
			mVertexBuffer_->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			stagingBuffer.setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.setupData(mPrimitives_->mVertices_.data(), bufferSize);
			VulkanBuffer::copyBuffer(mVulkanCommandPool_, stagingBuffer.Get().buffer, mVertexBuffer_->Get().buffer, bufferSize);

			mIndexBuffer_ = new VulkanBuffer();
			bufferSize = sizeof(mPrimitives_->mIndices_[0]) * mPrimitives_->mIndices_.size();
			mIndexBuffer_ = new VulkanBuffer();
			mIndexBuffer_->connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);
			mIndexBuffer_->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			stagingBuffer.setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.setupData(mPrimitives_->mIndices_.data(), bufferSize);
			VulkanBuffer::copyBuffer(mVulkanCommandPool_, stagingBuffer.Get().buffer, mIndexBuffer_->Get().buffer, bufferSize);
		}

		virtual void cleanup() override
		{
			SafeDestroy(mVertexBuffer_);
			SafeDestroy(mIndexBuffer_);
			SafeDestroy(mMaterial_);
		}

	public:
		// TODO: should be optimized, less pipeline change
		void draw(VkCommandBuffer commandBuffer)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mMaterial_->mGraphicsPipeline_->Get());

			VkBuffer vertexBuffers[] = { mVertexBuffer_->Get().buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer_->Get().buffer, 0, VK_INDEX_TYPE_UINT32);
			VkDescriptorSet set = mMaterial_->getDescriptorSet();
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mMaterial_->getPipelineLayout(), 0, 1, &set, 0, nullptr);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mPrimitives_->mIndices_.size()), 1, 0, 0, 0);
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