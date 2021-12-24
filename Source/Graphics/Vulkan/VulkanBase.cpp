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

namespace zyh
{
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
		case WM_KEYDOWN:
			switch (wParam)
			{
			case KEY_W:
			case KEY_S:
			case KEY_A:
			case KEY_D:
				mCamera_.handleInputKeyDown(uMsg);
				break;
			}

		case WM_KEYUP:
			switch (wParam)
			{
			case KEY_P:
				mIsPaused_ = !mIsPaused_;
				break;
			case KEY_ESCAPE:
				PostQuitMessage(0);
				break;

			case KEY_W:
			case KEY_S:
			case KEY_A:
			case KEY_D:
				mCamera_.handleInputKeyUp(uMsg);
				break;
			}

		case WM_LBUTTONDOWN:
			mCamera_.handleMouseButtonDown(LEFT);
			break;
		case WM_RBUTTONDOWN:
			mCamera_.handleMouseButtonDown(RIGHT);
			break;
		case WM_MBUTTONDOWN:
			mCamera_.handleMouseButtonDown(MID);
			break;
		case WM_LBUTTONUP:
			mCamera_.handleMouseButtonUp(LEFT);
			break;
		case WM_RBUTTONUP:
			mCamera_.handleMouseButtonUp(RIGHT);
			break;
		case WM_MBUTTONUP:
			mCamera_.handleMouseButtonUp(MID);
			break;
		case WM_MOUSEWHEEL:
			short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			mCamera_.handleMouseWheel(wheelDelta);
			break;
		case WM_MOUSEMOVE:
			mCamera_.handleMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_SIZE:
			windowResize();
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

	void VulkanBase::initVulkan()
	{
		mInstance_ = new VulkanInstance();
		mSurface_ = new VulkanSurface();
		mPhysicalDevice_ = new VulkanPhysicalDevice();
		mLogicalDevice_ = new VulkanLogicalDevice();
		mSwapchain_ = new VulkanSwapchain();
		mGraphicsCommandPool_ = new VulkanCommandPool(GRAPHICS);
		mDepthStencil_ = new VulkanImage();
		mRenderPass_ = new VulkanRenderPassBase();
		mGraphicsPipeline_ = new VulkanGraphicsPipeline();

		// connect
		mSurface_->connect(mInstance_);
		mPhysicalDevice_->connect(mInstance_, mSurface_);
		mLogicalDevice_->connect(mInstance_, mPhysicalDevice_);
		mSwapchain_->connect(mInstance_, mPhysicalDevice_, mLogicalDevice_);
		mGraphicsCommandPool_->connect(mPhysicalDevice_, mLogicalDevice_, mSwapchain_);
		mDepthStencil_->connect(mPhysicalDevice_, mLogicalDevice_);
		mRenderPass_->connect(mLogicalDevice_);
		mGraphicsPipeline_->connect(mLogicalDevice_, mRenderPass_);
	}

	void VulkanBase::setupVulkan()
	{
		uint32_t width = Setting::AppWidth;
		uint32_t height = Setting::AppHeight;

		mInstance_->setup();

#if defined(VK_USE_PLATFORM_WIN32_KHR)		
		mSurface_->setup(mWindowInstance_, mWindow_);
#endif

		mPhysicalDevice_->setup();
		
		mLogicalDevice_->setup();

		mSwapchain_->setup(&width, &height);
		
		mGraphicsCommandPool_->setup();
		
		mDepthStencil_->setup(
			width, height, 1,
			getMsaaSamples(), getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
		);
		
		mRenderPass_->setup(mSwapchain_->getColorFormat(), getMsaaSamples(), getDepthFormat());

		mGraphicsPipeline_->setup(
			"Resource/shaders/vert.spv",
			"Resource/shaders/frag.spv",
			mSwapchain_->getExtend(),
			getMsaaSamples()
		);

		std::vector<VkImageView> attachments = {
			mColorResources_->Get().view,
			mDepthResources_->Get().view,
		};
		mSwapchain_->setupFrameBuffer(*mRenderPass_, attachments);

		createSyncObjects();
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
		if (!mMsaaSamples_.IsValid())
		{
			*mMsaaSamples_ = mPhysicalDevice_->getMaxUsableSampleCount();
			mMsaaSamples_.IsValid(true);
		}
		return *mMsaaSamples_;
	}

	VkFormat VulkanBase::getDepthFormat()
	{
		if (!mDepthFromat_.IsValid())
		{
			*mDepthFromat_ = mPhysicalDevice_->findSupportedFormat(
				{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);
			mDepthFromat_.IsValid(true);
		}
		return *mDepthFromat_;
	}

	/// impl


	void VulkanBase::prepare()
	{
		createColorResources();

	}

	void VulkanBase::createColorResources()
	{
		mColorResources_ = new VulkanImage();
		mColorResources_.connect(mPhysicalDevice_, mLogicalDevice_);
		mColorResources_.setup(
			mSwapchain_->getExtend().width, mSwapchain_->getExtend().height, 1,
			getMsaaSamples(), mSwapchain_->getColorFormat(), VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	void VulkanBase::createDepthResources()
	{
		mDepthResources_ = new VulkanImage();
		mDepthResources_.connect(mPhysicalDevice_, mLogicalDevice_);
		mDepthResources_.setup(
			mSwapchain_->getExtend().width, mSwapchain_->getExtend().height, 1,
			getMsaaSamples(), mSwapchain_->getColorFormat(), VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
		);
	}

}
