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


namespace zyh
{
	VulkanBase* VulkanBase::GVulkanInstance = nullptr;

#if defined(_WIN32)
	HWND VulkanBase::setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
	{
		this->mWindowInstance_ = hinstance;

		WNDCLASSEX wndClass;

		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = wndproc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hinstance;
		wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = tools::stringToLPCWSTR(Setting::AppTitle);
		wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

		if (!RegisterClassEx(&wndClass))
		{
			std::cout << "Could not register window class!\n";
			fflush(stdout);
			exit(1);
		}

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		if (Setting::IsFullscreen)
		{
			if ((Setting::AppWidth != (uint32_t)screenWidth) && (Setting::AppHeight != (uint32_t)screenHeight))
			{
				DEVMODE dmScreenSettings;
				memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmPelsWidth = Setting::AppWidth;
				dmScreenSettings.dmPelsHeight = Setting::AppHeight;
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				{
					if (MessageBox(NULL, L"Full Screen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					{
						Setting::IsFullscreen = false;
					}
					else
					{
						return nullptr;
					}
				}
				screenWidth = Setting::AppWidth;
				screenHeight = Setting::AppHeight;
			}
		}

		DWORD dwExStyle;
		DWORD dwStyle;

		if (Setting::IsFullscreen)
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}
		else
		{
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}

		RECT windowRect;
		windowRect.left = 0L;
		windowRect.top = 0L;
		windowRect.right = Setting::IsFullscreen ? (long)screenWidth : (long)Setting::AppWidth;
		windowRect.bottom = Setting::IsFullscreen ? (long)screenHeight : (long)Setting::AppHeight;

		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

		std::string windowTitle = Setting::AppTitle;
		mWindow_ = CreateWindowEx(0,
			tools::stringToLPCWSTR(windowTitle),
			tools::stringToLPCWSTR(windowTitle),
			dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0,
			0,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			hinstance,
			NULL);

		if (!Setting::IsFullscreen)
		{
			// Center on screen
			uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
			uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
			SetWindowPos(mWindow_, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}

		if (!mWindow_)
		{
			printf("Could not create window!\n");
			fflush(stdout);
			return nullptr;
		}

		ShowWindow(mWindow_, SW_SHOW);
		SetForegroundWindow(mWindow_);
		SetFocus(mWindow_);

		return mWindow_;
	}

	LRESULT VulkanBase::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (VulkanBase::GVulkanInstance)
			VulkanBase::GVulkanInstance->handleMessage(hWnd, uMsg, wParam, lParam);
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}

	void VulkanBase::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CLOSE:
			mIsPaused_ = true;
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			mCamera_.handleInputKeyDown(wParam);
			break;

		case WM_KEYUP:
			mCamera_.handleInputKeyUp(wParam);
			switch (wParam)
			{
			case KEY_P:
				mIsPaused_ = !mIsPaused_;
				break;
			case KEY_ESCAPE:
				PostQuitMessage(0);
				break;
			case KEY_B:
				mCamera_.reset();
				break;
			}
			break;

		case WM_LBUTTONDOWN:
			mCamera_.handleMouseButtonDown(LEFT, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_RBUTTONDOWN:
			mCamera_.handleMouseButtonDown(RIGHT, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_MBUTTONDOWN:
			mCamera_.handleMouseButtonDown(MID, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_LBUTTONUP:
			mCamera_.handleMouseButtonUp(LEFT, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_RBUTTONUP:
			mCamera_.handleMouseButtonUp(RIGHT, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_MBUTTONUP:
			mCamera_.handleMouseButtonUp(MID, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_MOUSEWHEEL:
			mCamera_.handleMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
			break;
		case WM_MOUSEMOVE:
			mCamera_.handleMouseMove(LOWORD(lParam), HIWORD(lParam), mDeltaTime_);
			break;
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED) // minimized
			{
				mIsPaused_ = true; // just pause rendering when minimize screen
			}
			else
			{
				mIsPaused_ = false;
				if(mIsResizing_)
					windowResize(LOWORD(lParam), HIWORD(lParam));
			}
			break;
		case WM_ENTERSIZEMOVE:
			mIsResizing_ = true;
			break;
		case WM_EXITSIZEMOVE:
			mIsResizing_ = false;
			break;
		}
	}
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
		mDepthStencil_ = new VulkanImage();
		mRenderPass_ = new VulkanRenderPassBase("Resource/shaders/vert.spv", "Resource/shaders/frag.spv");
		mGraphicsPipeline_ = new VulkanGraphicsPipeline();

		// connect
		mSurface_->connect(mInstance_);
		mPhysicalDevice_->connect(mInstance_, mSurface_);
		mLogicalDevice_->connect(mInstance_, mPhysicalDevice_);
		mSwapchain_->connect(mInstance_, mPhysicalDevice_, mLogicalDevice_, mSurface_);
		mGraphicsCommandPool_->connect(mPhysicalDevice_, mLogicalDevice_, mSwapchain_);
		mDepthStencil_->connect(mPhysicalDevice_, mLogicalDevice_);
		mRenderPass_->connect(mLogicalDevice_);
		mGraphicsPipeline_->connect(mLogicalDevice_, mRenderPass_);
	}

	void VulkanBase::setupVulkan()
	{
		mWidth_ = Setting::AppWidth;
		mHeight_ = Setting::AppHeight;

		mInstance_->setup();

#if defined(VK_USE_PLATFORM_WIN32_KHR)		
		mSurface_->setup(mWindowInstance_, mWindow_);
#endif

		mPhysicalDevice_->setup();
		
		mLogicalDevice_->setup();

		mSwapchain_->setup(&mWidth_, &mHeight_);
		
		mGraphicsCommandPool_->setup();
		
		mDepthStencil_->setup(
			mWidth_, mHeight_, 1,
			getMsaaSamples(), getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
		);
		
		mRenderPass_->setup(mSwapchain_->getColorFormat(), getMsaaSamples(), getDepthFormat());

		mGraphicsPipeline_->setup();
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

		loadData();

		createCommandBuffers();
		createSyncObjects();

		mCamera_.mScreenHeight_ = static_cast<float>(mSwapchain_->getExtend().height);
		mCamera_.mScreenWidth_ = static_cast<float>(mSwapchain_->getExtend().width);
		mCamera_.updateProjMatrix();
	}

	void VulkanBase::Tick()
	{
		bool quitMessageReceived = false;
		while (!quitMessageReceived) {
#if defined(_WIN32)
			MSG msg;
			while (!quitMessageReceived) {
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					if (msg.message == WM_QUIT) {
						return;
					}
				}

				if (!mIsPaused_)
				{
					mCamera_.tick(mDeltaTime_);
					drawFrame();
				}
			}
#endif
			
		}

		vkDeviceWaitIdle(mLogicalDevice_->Get());
	}

	void VulkanBase::CleanUp()
	{
		vkDeviceWaitIdle(mLogicalDevice_->Get());

		for (auto& buffer : mCommandBuffers_)
			SafeDestroy(buffer);

		for(auto& buffer : mUniformBuffers_)
			SafeDestroy(buffer)

		SafeDestroy(mTextureImage_);
		SafeDestroy(mDepthResources_);
		SafeDestroy(mColorResources_);

		for (auto& element : mRenderElements_)
			SafeDestroy(element);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(mLogicalDevice_->Get(), mRenderFinishedSemaphores_[i], nullptr);
			vkDestroySemaphore(mLogicalDevice_->Get(), mImageAvailableSemaphores_[i], nullptr);
			vkDestroyFence(mLogicalDevice_->Get(), mInFlightFences_[i], nullptr);
		}
		SafeDestroy(mGraphicsPipeline_);
		SafeDestroy(mRenderPass_);
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

	void VulkanBase::createTextureImage()
	{
		mTextureImage_ = new VulkanTextureImage("Resource/textures/viking_room.png");
		mTextureImage_->connect(mPhysicalDevice_, mLogicalDevice_, mGraphicsCommandPool_);
		mTextureImage_->setup(VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	void VulkanBase::loadData()
	{
		createTextureImage();
		VulkanRenderElement* element;
		
		element = new VulkanRenderElement("Resource/models/viking_room.obj");
		element->connect(mPhysicalDevice_, mLogicalDevice_, mGraphicsCommandPool_);
		element->setup();
		mRenderElements_.push_back(element);

		element = new VulkanRenderElement("Resource/models/chair_low_chair.fbx");
		element->connect(mPhysicalDevice_, mLogicalDevice_, mGraphicsCommandPool_);
		element->setup();
		mRenderElements_.push_back(element);

		createUniformBuffer();
		createDescriptorSets();
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
			mCommandBuffers_[i] = new VulkanCommand();
			mCommandBuffers_[i]->connect(mLogicalDevice_, mGraphicsCommandPool_);
			mCommandBuffers_[i]->setup(allocInfo);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			mCommandBuffers_[i]->begin(&beginInfo);

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = mRenderPass_->Get();
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
			vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline_->Get());

			for (auto& renderElement : mRenderElements_)
				renderElement->draw(vkCommandBuffer);
			vkCmdEndRenderPass(vkCommandBuffer);
			
			mCommandBuffers_[i]->end();
		}
	}

	void VulkanBase::drawFrame()
	{
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

		updateUniformBuffer(imageIndex);

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
		mRenderPass_->setup(mSwapchain_->getColorFormat(), getMsaaSamples(), getDepthFormat());

		mGraphicsPipeline_->setup();
		createColorResources();
		createDepthResources();

		std::vector<VkImageView> attachments = {
			mColorResources_->Get().view,
			mDepthResources_->Get().view,
		};
		mSwapchain_->setupFrameBuffer(*mRenderPass_, attachments);
		
		createUniformBuffer();
		createDescriptorSets();
		createCommandBuffers();
	}

	void VulkanBase::_cleanupSwapchain()
	{
		mDepthStencil_->cleanup();
		mColorResources_->cleanup();
		
		for (auto& buffer : mCommandBuffers_)
			SafeDestroy(buffer);

		for (auto& buffer : mUniformBuffers_)
			SafeDestroy(buffer);
		
		mGraphicsPipeline_->cleanup();
		mRenderPass_->cleanup();
		mSwapchain_->cleanup();
		vkDestroyDescriptorPool(mLogicalDevice_->Get(), mDescriptorPool_, nullptr);
	}

}
