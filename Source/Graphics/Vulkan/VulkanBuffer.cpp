#include "VulkanBuffer.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanCommandPool.h"


namespace zyh
{

	void VulkanBuffer::createBuffer(
		VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice,
		VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
		VkBuffer& buffer, VkDeviceMemory& bufferMemory
	)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateBuffer(logicalDevice->Get(), &bufferInfo, nullptr, &buffer), "failed to create buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice->Get(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = physicalDevice->findMemoryType(memRequirements.memoryTypeBits, properties);

		VK_CHECK_RESULT(vkAllocateMemory(logicalDevice->Get(), &allocInfo, nullptr, &bufferMemory), "failed to allocate buffer memory!");
		vkBindBufferMemory(logicalDevice->Get(), buffer, bufferMemory, 0);
	}

	void VulkanBuffer::copyBuffer(VulkanCommandPool* commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		SingleTimeExecFunc func = [&](VulkanCommand& commandBuffer)
		{
			VkBufferCopy copyRegion{};
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer.Get(), srcBuffer, dstBuffer, 1, &copyRegion);
		};
		commandPool->generateSingleTimeCommand(func);
	}

	VulkanBuffer::~VulkanBuffer()
	{
		cleanup();
	}

	void VulkanBuffer::connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice)
	{
		mVulkanPhysicalDevice_ = physicalDevice;
		mVulkanLogicalDevice_ = logicalDevice;
	}

	void VulkanBuffer::setup(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		cleanup();
		_createBuffer(size, usage, properties);
	}

	void VulkanBuffer::setupData(void* data, VkDeviceSize size)
	{
		void* memData;
		vkMapMemory(mVulkanLogicalDevice_->Get(), mVkImpl_.mem, 0, size, 0, &memData);
		memcpy(memData, data, static_cast<size_t>(size));
		vkUnmapMemory(mVulkanLogicalDevice_->Get(), mVkImpl_.mem);
	}

	void VulkanBuffer::cleanup()
	{
		if (mVkImpl_.buffer)
		{
			vkDestroyBuffer(mVulkanLogicalDevice_->Get(), mVkImpl_.buffer, nullptr);
			mVkImpl_.buffer = VK_NULL_HANDLE;
		}

		if (mVkImpl_.mem)
		{
			vkFreeMemory(mVulkanLogicalDevice_->Get(), mVkImpl_.mem, nullptr);
			mVkImpl_.mem = VK_NULL_HANDLE;
		}
	}

	void VulkanBuffer::_createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		mBufferSize_ = size;
		createBuffer(mVulkanPhysicalDevice_, mVulkanLogicalDevice_, size, usage, properties, mVkImpl_.buffer, mVkImpl_.mem);
	}

}



