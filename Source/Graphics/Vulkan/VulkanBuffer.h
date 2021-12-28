#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanCommandPool;

	struct VulkanBufferCollection
	{
		VkBuffer buffer;
		VkDeviceMemory mem;
	};

	class VulkanBuffer : public TVulkanObject<VulkanBufferCollection>
	{
	public:
		static void createBuffer(
			VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice,
			VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
			VkBuffer& buffer, VkDeviceMemory& bufferMemory
		);
		static void copyBuffer(VulkanCommandPool* commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	public:
		virtual ~VulkanBuffer();

		void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice);
		void setup(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		void setupData(void* data, VkDeviceSize size);
		void cleanup();

	protected:
		void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

	private:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_;
		VulkanLogicalDevice* mVulkanLogicalDevice_;
	};
}