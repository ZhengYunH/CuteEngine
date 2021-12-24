#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;

	struct VulkanBufferCollection
	{
		VkBuffer buffer;
		VkDeviceMemory mem;
	};

	class VulkanBuffer : public TVulkanObject<VulkanBufferCollection>
	{
	public:
		virtual ~VulkanBuffer();

		void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice);
		void setup(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		void setupData(void* data, VkDeviceSize size);

	protected:
		void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

	private:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_;
		VulkanLogicalDevice* mVulkanLogicalDevice_;
	};
}