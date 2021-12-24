#include "VulkanBuffer.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"


namespace zyh
{
	VulkanBuffer::~VulkanBuffer()
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

	void VulkanBuffer::connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice)
	{
		mVulkanPhysicalDevice_ = physicalDevice;
		mVulkanLogicalDevice_ = logicalDevice;
	}

	void VulkanBuffer::setup(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		_createBuffer(size, usage, properties);
	}

	void VulkanBuffer::setupData(void* data, VkDeviceSize size)
	{
		void* data;
		vkMapMemory(mVulkanLogicalDevice_->Get(), mVkImpl_.mem, 0, size, 0, &data);
		memcpy(data, data, static_cast<size_t>(size));
		vkUnmapMemory(mVulkanLogicalDevice_->Get(), mVkImpl_.mem);
	}

	void VulkanBuffer::_createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateBuffer(mVulkanLogicalDevice_->Get(), &bufferInfo, nullptr, &mVkImpl_.buffer), "failed to create vertex buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mVulkanLogicalDevice_->Get(), mVkImpl_.buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = mVulkanPhysicalDevice_->findMemoryType(memRequirements.memoryTypeBits, properties);

		VK_CHECK_RESULT(vkAllocateMemory(mVulkanLogicalDevice_->Get(), &allocInfo, nullptr, &mVkImpl_.mem), "failed to allocate vertex buffer memory!");
		vkBindBufferMemory(mVulkanLogicalDevice_->Get(), mVkImpl_.buffer, mVkImpl_.mem, 0);
	}

}



