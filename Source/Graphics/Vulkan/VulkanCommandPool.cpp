#include "VulkanCommandPool.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapchain.h"

namespace zyh
{

	VulkanCommand::~VulkanCommand()
	{
		if (mVkImpl_)
		{
			if (mEndFunc_)
				mEndFunc_();
			vkFreeCommandBuffers(mVulkanLogicalDevice_->Get(), mVulkanCommandPool_->Get(), 1, &mVkImpl_);
			mVkImpl_ = VK_NULL_HANDLE;
		}
	}

	void VulkanCommand::connect(VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool)
	{
		mVulkanLogicalDevice_ = logicalDevice;
		mVulkanCommandPool_ = commandPool;
	}


	void VulkanCommand::setup(VkCommandBufferAllocateInfo allocInfo, std::function<void()> beginFunc /*= nullptr*/, std::function<void()> endFunc /*= nullptr*/)
	{
		vkAllocateCommandBuffers(mVulkanLogicalDevice_->Get(), &allocInfo, &mVkImpl_);
		if (beginFunc)
			beginFunc();
		if (endFunc)
			mEndFunc_ = endFunc;
	}

	void VulkanCommandPool::connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanSwapchain* swapchain)
	{
		mVulkanPhysicalDevice_ = physicalDevice;
		mVulkanLogicalDevice_ = logicalDevice;
		mVulkanSwapchain_ = swapchain;
	}

	void VulkanCommandPool::setup()
	{
		_setupCommandPool();
		_setupCommandBuffers();
	}


	void VulkanCommandPool::bindCommandBuffer()
	{

	}

	void VulkanCommandPool::_setupCommandPool()
	{
		QueueFamilyIndices indices = mVulkanPhysicalDevice_->findQueueFamilies();
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = indices->getIndexByQueueFamily(mQueueFamily_);
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(mVulkanLogicalDevice_->Get(), &poolInfo, nullptr/* Allocator*/, &mVkImpl_), "failed to create command pool!");
	}

	void VulkanCommandPool::_setupCommandBuffers()
	{
		mCommandBuffers_.resize(mVulkanSwapchain_->getImageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mVkImpl_;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)mCommandBuffers_.size();
		VK_CHECK_RESULT(vkAllocateCommandBuffers(mVulkanLogicalDevice_->Get(), &allocInfo, mCommandBuffers_.data()), "failed to allocate command buffers!");
	}

	VulkanCommand&& VulkanCommandPool::GenerateSingleTimeCommand()
	{
		VkQueue queue = mVulkanLogicalDevice_->graphicsQueue();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mVkImpl_;
		allocInfo.commandBufferCount = 1;

		VulkanCommand commandBuffer;
		commandBuffer.connect(mVulkanLogicalDevice_, this);

		std::function<void()> beginFunc = [&]() {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer.Get(), &beginInfo);
		};

		std::function<void()> endFunc = [&]() {
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer.Get();

			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(queue);
		};

		commandBuffer.setup(allocInfo, beginFunc, endFunc);
	}
}

