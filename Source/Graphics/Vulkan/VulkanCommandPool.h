#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanSwapchain;
	class VulkanCommandPool;

	enum E_QUEUE_FAMILY : uint8_t;

	class VulkanCommand : public TVulkanObject<VkCommandBuffer>
	{
	public:
		VulkanCommand() noexcept {}
		VulkanCommand(const VulkanCommand& rhs) noexcept;
		VulkanCommand(VulkanCommand&& rhs) noexcept;

		~VulkanCommand();
		void connect(VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool);
		void setup(VkCommandBufferAllocateInfo allocInfo);
		void cleanup() override;

	private:
		VulkanLogicalDevice* mVulkanLogicalDevice_{ nullptr };
		VulkanCommandPool* mVulkanCommandPool_{ nullptr };

	public:
		void begin(VkCommandBufferBeginInfo* beginInfo);
		void end();
	};

	typedef std::function<void(VulkanCommand&)> SingleTimeExecFunc;

	class VulkanCommandPool : public TVulkanObject<VkCommandPool>
	{
	public:
		VulkanCommandPool(E_QUEUE_FAMILY queueFamily) : mQueueFamily_{ queueFamily } {}
		void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanSwapchain* swapchain);
		void setup() override;
		void cleanup() override;
	
	private:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_{ nullptr };
		VulkanLogicalDevice* mVulkanLogicalDevice_{ nullptr };
		VulkanSwapchain* mVulkanSwapchain_{ nullptr };
		E_QUEUE_FAMILY mQueueFamily_;

	protected:
		void _setupCommandPool();
		void _setupCommandBuffers();

	public:
		void generateSingleTimeCommand(SingleTimeExecFunc execFunc);


	private:
		std::vector<VkCommandBuffer> mCommandBuffers_;
	};
}