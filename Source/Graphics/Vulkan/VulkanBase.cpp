#include "Core/Engine.h"
#include "Core/ClientScene.h"

#include "VulkanBase.h"
#include "VulkanTools.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanImage.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"

#include "VulkanRenderElement.h"

#include "Graphics/Common/ResourceLoader.h"
#include "Common/TestData.h"
#include "Core/InputSystem.h"
#include "Graphics/Common/IRenderPass.h"
#include "Camera/Camera.h"


#include "Graphics/Imgui/imgui.h"
#include "Graphics/Imgui/imgui_impl_vulkan.h"
#include "Graphics/Imgui/imgui_impl_win32.h"


namespace zyh
{
	VulkanBase* GVulkanInstance = nullptr;

	VulkanBase::~VulkanBase()
	{
	}

	void VulkanBase::initVulkan()
	{
		GVulkanInstance = this;

		mInstance_ = new VulkanInstance();
		mSurface_ = new VulkanSurface();
		mPhysicalDevice_ = new VulkanPhysicalDevice();
		mLogicalDevice_ = new VulkanLogicalDevice();
		mSwapchain_ = new VulkanSwapchain();
		mGraphicsCommandPool_ = new VulkanCommandPool(GRAPHICS);
		mFrameBufferRenderPass_ = new VulkanRenderPass(new IRenderPass("FrameBuffer", { RenderSet::SCENE }));

		// connect
		mSurface_->connect(mInstance_);
		mPhysicalDevice_->connect(mInstance_, mSurface_);
		mLogicalDevice_->connect(mInstance_, mPhysicalDevice_);
		mSwapchain_->connect(mInstance_, mPhysicalDevice_, mLogicalDevice_, mSurface_);
		mGraphicsCommandPool_->connect(mPhysicalDevice_, mLogicalDevice_, mSwapchain_);
	}

	void VulkanBase::setupVulkan()
	{
		mWidth_ = Setting::AppWidth;
		mHeight_ = Setting::AppHeight;

		mInstance_->setup();

#if defined(VK_USE_PLATFORM_WIN32_KHR)		
		mSurface_->setup(GEngine->mWindowInstance_, GEngine->mWindow_);
#endif

		mPhysicalDevice_->setup();
		
		mLogicalDevice_->setup();

		mSwapchain_->setup(&mWidth_, &mHeight_);
		
		mGraphicsCommandPool_->setup();

		RenderTarget target(
			mSwapchain_->getExtend().width,
			mSwapchain_->getExtend().height,
			1,
			ETextureType::Texture2D,
			EPixelFormat::R8G8B8A8
		);
		target.Quality = ESamplerQuality::Quality8X;

		RenderTarget depthStencil(
			mSwapchain_->getExtend().width,
			mSwapchain_->getExtend().height,
			1,
			ETextureType::Texture2D,
			EPixelFormat::D32_SFLOAT_S8_UINT
		);
		depthStencil.Quality = ESamplerQuality::Quality8X;


		mFrameBufferRenderPass_->GetRenderPass()->AddRenderTarget(target);
		mFrameBufferRenderPass_->GetRenderPass()->SetDepthStencilTarget(depthStencil);
		mFrameBufferRenderPass_->InitailizeResource();
	}

	void VulkanBase::createSyncObjects()
	{
		mImageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
		mImagesInFights_.resize(mSwapchain_->getImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK_RESULT(vkCreateSemaphore(mLogicalDevice_->Get(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores_[i]), "failed to create image available semaphores!");
			VK_CHECK_RESULT(vkCreateSemaphore(mLogicalDevice_->Get(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores_[i]), "failed to create render finished semaphores!");
			VK_CHECK_RESULT(vkCreateFence(mLogicalDevice_->Get(), &fenceInfo, nullptr, &mInFlightFences_[i]), "failed to create fence!");
		}
	}

	VkSampleCountFlagBits VulkanBase::getMsaaSamples()
	{
		HYBRID_CHECK(GInstance->mMsaaSamples_.IsValid());
		return *GInstance->mMsaaSamples_;
	}

	VkFormat VulkanBase::getDepthFormat()
	{
		HYBRID_CHECK(GInstance->mDepthFormat_.IsValid());
		return *GInstance->mDepthFormat_;
	}

	/// impl
	void VulkanBase::prepare()
	{
		mSwapchain_->setupFrameBuffer(*mFrameBufferRenderPass_);
		createCommandBuffers();
		createSyncObjects();

		GEngine->Scene->GetCamera()->mScreenHeight_ = static_cast<float>(mSwapchain_->getExtend().height);
		GEngine->Scene->GetCamera()->mScreenWidth_ = static_cast<float>(mSwapchain_->getExtend().width);
		GEngine->Scene->GetCamera()->updateProjMatrix();
	}

	void VulkanBase::CleanUp()
	{
		vkDeviceWaitIdle(mLogicalDevice_->Get());

		for (auto& buffers : mCommandBuffers_)
		{
			for (auto& buffer : buffers)
			{
				SafeDestroy(buffer);
			}
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(mLogicalDevice_->Get(), mRenderFinishedSemaphores_[i], nullptr);
			vkDestroySemaphore(mLogicalDevice_->Get(), mImageAvailableSemaphores_[i], nullptr);
			vkDestroyFence(mLogicalDevice_->Get(), mInFlightFences_[i], nullptr);
		}
		SafeDestroy(mSwapchain_);
		SafeDestroy(mLogicalDevice_);
		SafeDestroy(mPhysicalDevice_);
		SafeDestroy(mSurface_);
		SafeDestroy(mInstance_);
	}

	void VulkanBase::windowResize(uint32_t width, uint32_t height)
	{
		mWidth_ = width;
		mHeight_ = height;
		recreateSwapchain();
	}

	void VulkanBase::createCommandBuffers()
	{
		mCommandBuffers_.resize(mSwapchain_->getImageCount());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mGraphicsCommandPool_->Get();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (size_t i = 0; i < mCommandBuffers_.size(); ++i)
		{
			mCommandBuffers_[i] = std::vector<VulkanCommand*>();
			auto& lastBuffers = mCommandBuffers_[i];

			for (size_t j = 0; j < 1; ++j)
			{
				VulkanCommand* buffer = createCommandBuffer();
				lastBuffers.push_back(buffer);
			}
		}
	}

	VulkanCommand* VulkanBase::createCommandBuffer(VkCommandBufferAllocateInfo* pAllocInfo /*= nullptr*/)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		if (pAllocInfo)
		{
			allocInfo = *pAllocInfo;
		}
		else // default allocate info
		{
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = mGraphicsCommandPool_->Get();
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;
		}
		VulkanCommand* buffer = new VulkanCommand();
		buffer->connect(mLogicalDevice_, mGraphicsCommandPool_);
		buffer->setup(allocInfo);
		return buffer;
	}

	void VulkanBase::drawFrame()
	{
		vkWaitForFences(mLogicalDevice_->Get(), 1, &mInFlightFences_[mCurrentFrame_], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex = static_cast<int32_t>(mCurrentImage_);
		VkResult result;

		// Submitting the command buffer
		std::vector<VkCommandBuffer> commandList = {};
		for (size_t i = 0; i < mCommandBuffers_[mCurrentImage_].size(); ++i)
		{
			commandList.push_back(mCommandBuffers_[mCurrentImage_][i]->Get());
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = static_cast<uint32_t>(commandList.size());
		submitInfo.pCommandBuffers = commandList.data();

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores_[mCurrentFrame_] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores_[mCurrentFrame_] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(mLogicalDevice_->Get(), 1, &mInFlightFences_[mCurrentFrame_]);
		VK_CHECK_RESULT(vkQueueSubmit(mLogicalDevice_->graphicsQueue(), 1, &submitInfo, mInFlightFences_[mCurrentFrame_]), "failed to submit draw command buffer!");

		mImagesInFights_[mCurrentImage_] = mInFlightFences_[mCurrentFrame_];

		// Presentation
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapchain_->Get() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(mLogicalDevice_->presentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFrameBufferResized_) {
			mFrameBufferResized_ = false;
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		mCurrentFrame_ = (mCurrentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanBase::recreateSwapchain()
	{
		// Ensure all operations on the device have been finished before destroying resources
		vkDeviceWaitIdle(mLogicalDevice_->Get());
		_cleanupSwapchain();

		// Recreate swap chain
		mSwapchain_->setup(&mWidth_, &mHeight_);
		mSwapchain_->setupFrameBuffer(*mFrameBufferRenderPass_);
		
		createCommandBuffers();
	}

	void VulkanBase::_cleanupSwapchain()
	{
		for (auto& buffers : mCommandBuffers_)
		{
			for (auto& buffer : buffers)
			{
				SafeDestroy(buffer);
			}
		}
			
		mSwapchain_->cleanup();
	}

	VulkanCommand* VulkanBase::GetCommandBuffer()
	{
		if (mFreeCommandBufferIdx_ >= mCommandBuffers_[mCurrentImage_].size())
		{
			mCommandBuffers_[mCurrentImage_].push_back(createCommandBuffer());
		}
		VulkanCommand* cmd = mCommandBuffers_[mCurrentImage_][mFreeCommandBufferIdx_];
		mFreeCommandBufferIdx_++;
		HYBRID_CHECK(mFreeCommandBufferIdx_ <= mCommandBuffers_[mCurrentImage_].size());
		return cmd;
	}

	VkFramebuffer VulkanBase::GetSwapchainFrameBuffer()
	{
		return mSwapchain_->getFrameBuffer(static_cast<uint32_t>(mCurrentImage_));
	}

	void VulkanBase::DrawFrameBegin(size_t& OutCurrentImage)
	{
		// Acquiring an image from the swap chain
		{
			uint32_t imageIndex;

			// acquire next image we want to use
			mSwapchain_->acquireNextImage(mImageAvailableSemaphores_[mCurrentFrame_], &imageIndex);

			// Check if a previous frame is using this image (if true, wait for it)
			if (mImagesInFights_[imageIndex] != VK_NULL_HANDLE) {
				vkWaitForFences(mLogicalDevice_->Get(), 1, &mImagesInFights_[imageIndex], VK_TRUE, UINT64_MAX);
			}
			mCurrentImage_ = imageIndex;
		}

		mFreeCommandBufferIdx_ = 0;
		OutCurrentImage = mCurrentImage_;
	}
}
