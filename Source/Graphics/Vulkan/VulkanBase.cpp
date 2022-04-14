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


namespace zyh
{
	VulkanBase* VulkanBase::GVulkanInstance = nullptr;

#if defined(_WIN32)
	//void VulkanBase::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	//{
	//	

	//	//switch (uMsg)
	//	//{
	//	//case WM_CLOSE:
	//	//	mIsPaused_ = true;
	//	//	DestroyWindow(hWnd);
	//	//	PostQuitMessage(0);
	//	//	break;
	//	//case WM_KEYDOWN:
	//	//	break;

	//	//case WM_KEYUP:
	//	//	switch (wParam)
	//	//	{
	//	//	case KEY_P:
	//	//		mIsPaused_ = !mIsPaused_;
	//	//		break;
	//	//	case KEY_ESCAPE:
	//	//		PostQuitMessage(0);
	//	//		break;
	//	//	case KEY_B:
	//	//		break;
	//	//	}
	//	//	break;

	//	//case WM_SIZE:
	//	//	if (wParam == SIZE_MINIMIZED) // minimized
	//	//	{
	//	//		mIsPaused_ = true; // just pause rendering when minimize screen
	//	//	}
	//	//	else
	//	//	{
	//	//		mIsPaused_ = false;
	//	//		if(mIsResizing_)
	//	//			windowResize(LOWORD(lParam), HIWORD(lParam));
	//	//	}
	//	//	break;
	//	//case WM_ENTERSIZEMOVE:
	//	//	mIsResizing_ = true;
	//	//	break;
	//	//case WM_EXITSIZEMOVE:
	//	//	mIsResizing_ = false;
	//	//	break;
	//	//}
	//}
#endif

	VulkanBase::~VulkanBase()
	{

	}

	void VulkanBase::initVulkan()
	{
		mInstance_ = new VulkanInstance();
		mSurface_ = new VulkanSurface();
		mPhysicalDevice_ = new VulkanPhysicalDevice();
		mLogicalDevice_ = new VulkanLogicalDevice();
		mSwapchain_ = new VulkanSwapchain();
		mGraphicsCommandPool_ = new VulkanCommandPool(GRAPHICS);
		mRenderPass_ = new VulkanRenderPassBase("Resource/shaders/vert.spv", "Resource/shaders/frag.spv");
		mDepthStencil_ = new VulkanImage();

		// connect
		mSurface_->connect(mInstance_);
		mPhysicalDevice_->connect(mInstance_, mSurface_);
		mLogicalDevice_->connect(mInstance_, mPhysicalDevice_);
		mSwapchain_->connect(mInstance_, mPhysicalDevice_, mLogicalDevice_, mSurface_);
		mGraphicsCommandPool_->connect(mPhysicalDevice_, mLogicalDevice_, mSwapchain_);
		mRenderPass_->connect(mLogicalDevice_);
		mDepthStencil_->connect(mPhysicalDevice_, mLogicalDevice_);
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

		mRenderPass_->setup(mSwapchain_->getColorFormat(), getMsaaSamples(), getDepthFormat());
		AddRenderPass(mRenderPass_);

		mDepthStencil_->setup(
			mWidth_, mHeight_, 1,
			getMsaaSamples(), getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
		);
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
		createColorResources();
		createDepthResources();

		std::vector<VkImageView> attachments = {
			mColorResources_->Get().view,
			mDepthResources_->Get().view,
		};
		mSwapchain_->setupFrameBuffer(*mRenderPass_, attachments);

		createCommandBuffers();
		createSyncObjects();

		GEngine->Scene->GetCamera()->mScreenHeight_ = static_cast<float>(mSwapchain_->getExtend().height);
		GEngine->Scene->GetCamera()->mScreenWidth_ = static_cast<float>(mSwapchain_->getExtend().width);
		GEngine->Scene->GetCamera()->updateProjMatrix();
	}

	void VulkanBase::Tick()
	{
		drawFrame();

/*
		bool quitMessageReceived = false;
		while (!quitMessageReceived) {
			if (!mIsPaused_)
			{
				drawFrame();
			}
		}
*/
	}

	void VulkanBase::CleanUp()
	{
		vkDeviceWaitIdle(mLogicalDevice_->Get());

		for (auto& buffer : mCommandBuffers_)
			SafeDestroy(buffer);

		SafeDestroy(mDepthResources_);
		SafeDestroy(mColorResources_);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(mLogicalDevice_->Get(), mRenderFinishedSemaphores_[i], nullptr);
			vkDestroySemaphore(mLogicalDevice_->Get(), mImageAvailableSemaphores_[i], nullptr);
			vkDestroyFence(mLogicalDevice_->Get(), mInFlightFences_[i], nullptr);
		}
		SafeDestroy(mDepthStencil_);
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

	void VulkanBase::createColorResources()
	{
		mColorResources_ = new VulkanImage();
		mColorResources_->connect(mPhysicalDevice_, mLogicalDevice_);
		mColorResources_->setup(
			mSwapchain_->getExtend().width, mSwapchain_->getExtend().height, 1,
			getMsaaSamples(), mSwapchain_->getColorFormat(), VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	void VulkanBase::createDepthResources()
	{
		mDepthResources_ = new VulkanImage();
		mDepthResources_->connect(mPhysicalDevice_, mLogicalDevice_);
		mDepthResources_->setup(
			mSwapchain_->getExtend().width, mSwapchain_->getExtend().height, 1,
			getMsaaSamples(), getDepthFormat(), VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
		);
	}

	void VulkanBase::createCommandBuffers()
	{
		GEngine->Scene->CollectAllRenderElements();
		for (auto& renderElement : GEngine->Scene->GetRenderElements(RenderSet::SCENE))
		{
			VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
			element->connect(mPhysicalDevice_, mLogicalDevice_, mGraphicsCommandPool_);
			element->setup();
		}

		mCommandBuffers_.resize(mSwapchain_->getImageCount());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mGraphicsCommandPool_->Get();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (size_t i = 0; i < mCommandBuffers_.size(); ++i)
		{
			mCommandBuffers_[i] = new VulkanCommand();
			mCommandBuffers_[i]->connect(mLogicalDevice_, mGraphicsCommandPool_);
			mCommandBuffers_[i]->setup(allocInfo);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			mCommandBuffers_[i]->begin(&beginInfo);

			for (auto renderPass : mRenderPasses_)
			{
				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass->Get();
				renderPassInfo.framebuffer = mSwapchain_->getFrameBuffer(static_cast<uint32_t>(i));
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = mSwapchain_->getExtend();

				std::array<VkClearValue, 2> clearValues{};
				clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
				clearValues[1].depthStencil = { 1.0f, 0 };

				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();

				VkCommandBuffer vkCommandBuffer = mCommandBuffers_[i]->Get();

				vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				for (auto& renderElement : GEngine->Scene->GetRenderElements(RenderSet::SCENE))
				{
					VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
					element->draw(vkCommandBuffer);
				}
				vkCmdEndRenderPass(vkCommandBuffer);

				mCommandBuffers_[i]->end();
			}
		}
	}

	void VulkanBase::drawFrame()
	{
		GEngine->Scene->CollectAllRenderElements();
		for (auto& renderElement : GEngine->Scene->GetRenderElements(RenderSet::SCENE))
		{
			VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
			element->mMaterial_->updateUniformBuffer(mCurrentFrame_);
		}

		vkWaitForFences(mLogicalDevice_->Get(), 1, &mInFlightFences_[mCurrentFrame_], VK_TRUE, UINT64_MAX);

		// Acquiring an image from the swap chain
		uint32_t imageIndex;
		VkResult result = mSwapchain_->acquireNextImage(mImageAvailableSemaphores_[mCurrentFrame_], &imageIndex);

		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (mImagesInFights_[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(mLogicalDevice_->Get(), 1, &mImagesInFights_[imageIndex], VK_TRUE, UINT64_MAX);
		}
		// Mark the image as now being in use by this frame
		mImagesInFights_[imageIndex] = mImagesInFights_[mCurrentFrame_];

		// Submitting the command buffer
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores_[mCurrentFrame_] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers_[imageIndex]->Get();

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores_[mCurrentFrame_] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(mLogicalDevice_->Get(), 1, &mInFlightFences_[mCurrentFrame_]);
		VK_CHECK_RESULT(vkQueueSubmit(mLogicalDevice_->graphicsQueue(), 1, &submitInfo, mInFlightFences_[mCurrentFrame_]), "failed to submit draw command buffer!");

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

		createColorResources();
		createDepthResources();

		std::vector<VkImageView> attachments = {
			mColorResources_->Get().view,
			mDepthResources_->Get().view,
		};
		mSwapchain_->setupFrameBuffer(*mRenderPass_, attachments);
		
		createCommandBuffers();
	}

	void VulkanBase::_cleanupSwapchain()
	{
		mDepthStencil_->cleanup();
		mColorResources_->cleanup();
		
		for (auto& buffer : mCommandBuffers_)
			SafeDestroy(buffer);
		
		mSwapchain_->cleanup();
	}

}
