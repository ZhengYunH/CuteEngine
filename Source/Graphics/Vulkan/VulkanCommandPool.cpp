#include "VulkanCommandPool.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapchain.h"

namespace zyh
{
	VulkanCommand::VulkanCommand(const VulkanCommand& rhs) noexcept
	{
		mVkImpl_ = rhs.mVkImpl_;
		mVulkanCommandPool_ = rhs.mVulkanCommandPool_;
		mVulkanLogicalDevice_ = rhs.mVulkanLogicalDevice_;
	}

	VulkanCommand::VulkanCommand(VulkanCommand&& rhs) noexcept
	{
		mVkImpl_ = rhs.mVkImpl_;
		mVulkanCommandPool_ = rhs.mVulkanCommandPool_;
		mVulkanLogicalDevice_ = rhs.mVulkanLogicalDevice_;

		rhs.mVkImpl_ = VK_NULL_HANDLE;
	}

	VulkanCommand::~VulkanCommand()
	{
		if (mVkImpl_)
		{
			cleanup();
			mVkImpl_ = VK_NULL_HANDLE;
		}
	}

	void VulkanCommand::connect(VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool)
	{
		mVulkanLogicalDevice_ = logicalDevice;
		mVulkanCommandPool_ = commandPool;
	}


	void VulkanCommand::setup(VkCommandBufferAllocateInfo allocInfo)
	{
		vkAllocateCommandBuffers(mVulkanLogicalDevice_->Get(), &allocInfo, &mVkImpl_);
	}

	void VulkanCommand::cleanup()
	{
		vkFreeCommandBuffers(mVulkanLogicalDevice_->Get(), mVulkanCommandPool_->Get(), 1, &mVkImpl_);
		mVkImpl_ = VK_NULL_HANDLE;
	}

	void VulkanCommand::begin(VkCommandBufferBeginInfo* beginInfo)
	{
		VK_CHECK_RESULT(vkBeginCommandBuffer(mVkImpl_, beginInfo), "failed to begin recording command buffer!");
	}

	void VulkanCommand::end()
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(mVkImpl_), "failed to record command buffer!");
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

	void VulkanCommandPool::cleanup()
	{
		vkDestroyCommandPool(mVulkanLogicalDevice_->Get(), mVkImpl_, nullptr);
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

	void VulkanCommandPool::generateSingleTimeCommand(SingleTimeExecFunc execFunc)
	{
		VkQueue queue = mVulkanLogicalDevice_->graphicsQueue();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mVkImpl_;
		allocInfo.commandBufferCount = 1;

		VulkanCommand commandBuffer;
		commandBuffer.connect(mVulkanLogicalDevice_, this);
		commandBuffer.setup(allocInfo);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		commandBuffer.begin(&beginInfo);
		execFunc(commandBuffer);
		commandBuffer.end();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer.Get();

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue); 
	}
}

