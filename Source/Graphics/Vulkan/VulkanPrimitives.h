#pragma once
#include "Graphics/Common/IPrimitive.h"
#include "IVulkanObject.h"
#include "VulkanBuffer.h"

namespace zyh
{
	class VulkanMaterial;

	class VulkanPrimitives : public IPrimitive, public IVulkanObject
	{
	public:
		/// <summary>
		/// Implementation of IVulkanObject
		/// </summary>
		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool)
		{
			mVulkanPhysicalDevice_ = physicalDevice;
			mVulkanLogicalDevice_ = logicalDevice;
			mVulkanCommandPool_ = commandPool;
		}

		virtual void setup() override
		{
			// TODO: Global stagingBuffer
			VulkanBuffer stagingBuffer;
			stagingBuffer.connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);

			mVertexBuffer_ = new VulkanBuffer();
			// create Vertex Buffer
			VkDeviceSize bufferSize = sizeof(mVertices_[0]) * mVertices_.size();
			mVertexBuffer_ = new VulkanBuffer();
			mVertexBuffer_->connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);
			mVertexBuffer_->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			stagingBuffer.setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.setupData(mVertices_.data(), bufferSize);
			VulkanBuffer::copyBuffer(mVulkanCommandPool_, stagingBuffer.Get().buffer, mVertexBuffer_->Get().buffer, bufferSize);

			mIndexBuffer_ = new VulkanBuffer();
			bufferSize = sizeof(mIndices_[0]) * mIndices_.size();
			mIndexBuffer_ = new VulkanBuffer();
			mIndexBuffer_->connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);
			mIndexBuffer_->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			stagingBuffer.setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.setupData(mIndices_.data(), bufferSize);
			VulkanBuffer::copyBuffer(mVulkanCommandPool_, stagingBuffer.Get().buffer, mIndexBuffer_->Get().buffer, bufferSize);
		}

		virtual void cleanup() override
		{
			SafeDestroy(mVertexBuffer_);
			SafeDestroy(mIndexBuffer_);
		}

	public:
		void draw(VkCommandBuffer commandBuffer)
		{
			VkBuffer vertexBuffers[] = { mVertexBuffer_->Get().buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer_->Get().buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VulkanMaterial*>(mMaterial_)->getPipelineLayout(), 0, 1, static_cast<VulkanMaterial*>(mMaterial_)->getDescriptorSet(), 0, nullptr);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mIndices_.size()), 1, 0, 0, 0);
		}

	protected:
		VulkanBuffer* mVertexBuffer_;
		VulkanBuffer* mIndexBuffer_;

	protected:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_;
		VulkanLogicalDevice* mVulkanLogicalDevice_;
		VulkanCommandPool* mVulkanCommandPool_;
	};
}