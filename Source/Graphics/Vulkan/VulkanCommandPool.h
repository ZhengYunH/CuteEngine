#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanSwapchain;
	class VulkanCommandPool;

	extern enum E_QUEUE_FAMILY : uint8_t;

	class VulkanCommand : public TVulkanObject<VkCommandBuffer>
	{
	public:
		~VulkanCommand();
		void connect(VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool);
		void setup(VkCommandBufferAllocateInfo allocInfo, std::function<void()> beginFunc = nullptr, std::function<void()> endFunc = nullptr);

	private:
		VulkanLogicalDevice* mVulkanLogicalDevice_{ nullptr };
		VulkanCommandPool* mVulkanCommandPool_{ nullptr };
		std::function<void()> mEndFunc_{ nullptr };
	};

	class VulkanCommandPool : public TVulkanObject<VkCommandPool>
	{
	public:
		VulkanCommandPool(E_QUEUE_FAMILY queueFamily) : mQueueFamily_{ queueFamily } {}
		void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanSwapchain* swapchain);
		void setup() override;
		void bindCommandBuffer();
	
	private:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_{ nullptr };
		VulkanLogicalDevice* mVulkanLogicalDevice_{ nullptr };
		VulkanSwapchain* mVulkanSwapchain_{ nullptr };
		E_QUEUE_FAMILY mQueueFamily_;

	protected:
		void _setupCommandPool();
		void _setupCommandBuffers();

	public:
		VulkanCommand&& GenerateSingleTimeCommand();


	private:
		std::vector<VkCommandBuffer> mCommandBuffers_;
	};
}