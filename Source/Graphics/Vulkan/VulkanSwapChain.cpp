#include "VulkanSwapChain.h"

namespace zyh
{

	void VulkanSwapChain::initSurface(void* platformHandle, void* platformWindow)
	{
		VkResult err = VK_SUCCESS;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = (HINSTANCE)platformHandle;
		surfaceCreateInfo.hwnd = (HWND)platformWindow;
		err = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.window = window;
		err = vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, NULL, &surface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
		VkIOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
		surfaceCreateInfo.pNext = NULL;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.pView = view;
		err = vkCreateIOSSurfaceMVK(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
		VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
		surfaceCreateInfo.pNext = NULL;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.pView = view;
		err = vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, NULL, &surface);
#elif defined(_DIRECT2DISPLAY)
		createDirect2DisplaySurface(width, height);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
		VkDirectFBSurfaceCreateInfoEXT surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
		surfaceCreateInfo.dfb = dfb;
		surfaceCreateInfo.surface = window;
		err = vkCreateDirectFBSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.display = display;
		surfaceCreateInfo.surface = window;
		err = vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.connection = connection;
		surfaceCreateInfo.window = window;
		err = vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
		VkHeadlessSurfaceCreateInfoEXT surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
		PFN_vkCreateHeadlessSurfaceEXT fpCreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateHeadlessSurfaceEXT");
		if (!fpCreateHeadlessSurfaceEXT) {
			vks::tools::exitFatal("Could not fetch function pointer for the headless extension!", -1);
		}
		err = fpCreateHeadlessSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &surface);
#endif
		if (err != VK_SUCCESS) {
			zyh::tools::exitFatal("Could not create surface!", err);
		}
	}

	void VulkanSwapChain::connect(VkInstance instance, VkPhysicalDevice physicsDevice, VkDevice device)
	{
		mInstance_ = instance;
		mPhysicalDevice_ = physicsDevice;
		mDevice_ = device;

		fpGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
		fpGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
		fpGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
		fpGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

		fpCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR"));
		fpDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR"));
		fpGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR"));
		fpAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR"));
		fpQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(device, "vkQueuePresentKHR"));
	}

	void VulkanSwapChain::create(uint32_t* width, uint32_t* height, bool vsync /*= false*/)
	{
		HYBRID_CHECK(mInstance_);
		
		VK_CHECK_RESULT(_createSwapChain(width, height, vsync));
		_destroySwapChain(swapChain, buffers);
		VK_CHECK_RESULT(_createSwapChainImages(width, height, vsync));
	}

	void VulkanSwapChain::cleanup()
	{

	}

	const SwapChainSupportDetails& VulkanSwapChain::querySwapChainSupport()
	{
		if (!mSwapChainSupportDetails_.isValid)
		{
			HYBRID_CHECK(mPhysicalDevice_);
			HYBRID_CHECK(mSurface_);

			SwapChainSupportDetails& details = mSwapChainSupportDetails_;

			// simply query capabilities
			fpGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice_, mSurface_, &details.capabilities);

			// query the supported surface formats
			uint32_t formatCount;
			fpGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice_, mSurface_, &formatCount, nullptr);
			if (formatCount > 0) {
				details.formats.resize(formatCount);
				fpGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice_, mSurface_, &formatCount, details.formats.data());
			}

			// query the supported presentation modes
			uint32_t presentModeCount;
			fpGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice_, mSurface_, &presentModeCount, nullptr);
			if (presentModeCount > 0) {
				details.presentModes.resize(presentModeCount);
				fpGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice_, mSurface_, &presentModeCount, details.presentModes.data());
			}

			mSwapChainSupportDetails_.isValid = true;
		}
		return mSwapChainSupportDetails_;
	}

	VkResult VulkanSwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex)
	{
		return fpAcquireNextImageKHR(mDevice_, swapChain, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
	}

	VkResult VulkanSwapChain::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore /*= VK_NULL_HANDLE*/)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;
		}
		return fpQueuePresentKHR(queue, &presentInfo);
	}

	VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync /*= false*/)
	{
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

		if (!vsync)
		{
			for (const auto& availablePresentMode : availablePresentModes) 
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
				{
					presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
				{
					presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}
		
		return presentMode;
	}

	VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t* width, uint32_t* height)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			*width = capabilities.currentExtent.width;
			*height = capabilities.currentExtent.height;
		}
		else {
			VkExtent2D actualExtent = { *width, *height };

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	VkSurfaceTransformFlagBitsKHR VulkanSwapChain::choosePreTransform(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			// We prefer a non-rotated transform
			return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			return capabilities.currentTransform;
		}
	}

	VkCompositeAlphaFlagBitsKHR VulkanSwapChain::chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags) {
			if (capabilities.supportedCompositeAlpha & compositeAlphaFlag) {
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		return compositeAlpha;
	}

	uint32_t VulkanSwapChain::getMinImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		uint32_t desiredNumberOfSwapchainImages = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
			desiredNumberOfSwapchainImages = capabilities.maxImageCount;
		}
		return desiredNumberOfSwapchainImages;
	}

	VkResult VulkanSwapChain::_createSwapChain(uint32_t* width, uint32_t* height, bool vsync /*= false*/)
	{
		VkSwapchainKHR oldSwapchain = swapChain;

		const SwapChainSupportDetails& swapChainSupport = querySwapChainSupport();
		const VkSurfaceCapabilitiesKHR& surfCaps = swapChainSupport.capabilities;

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface_;
		createInfo.minImageCount = getMinImageCount(surfCaps);
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = chooseSwapExtent(surfCaps, width, height);
		createInfo.imageArrayLayers = 1;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)choosePreTransform(surfCaps);
		// Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes sure that we can still present already acquired images
		createInfo.oldSwapchain = oldSwapchain;
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		createInfo.clipped = VK_TRUE;
		createInfo.compositeAlpha = chooseCompositeAlpha(surfCaps);

		// Enable transfer source on swap chain images if supported
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		// Enable transfer destination on swap chain images if supported
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		return fpCreateSwapchainKHR(mDevice_, &createInfo, nullptr, &swapChain);
	}
	
	VkResult VulkanSwapChain::_createSwapChainImages()
	{
		VK_CHECK_RESULT(fpGetSwapchainImagesKHR(mDevice_, swapChain, &imageCount, NULL));
		images.resize(imageCount);
		VK_CHECK_RESULT(fpGetSwapchainImagesKHR(mDevice_, swapChain, &imageCount, images.data()));

		const SwapChainSupportDetails& swapChainSupport = querySwapChainSupport();
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

		buffers.resize(imageCount);
		for (size_t i = 0; i < imageCount; i++) {
			buffers[i].image = images[i];
			buffers[i].view = _createImageView(buffers[i].image, surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	void VulkanSwapChain::_destroySwapChain(VkSwapchainKHR& swapChain, std::vector<SwapChainBuffer>& buffers)
	{
		if (swapChain != VK_NULL_HANDLE) 
		{
			for (uint32_t i = 0; i < buffers.size(); ++i)
			{
				vkDestroyImageView(mDevice_, buffers[i].view, nullptr);
			}
			fpDestroySwapchainKHR(mDevice_, swapChain, nullptr);
		}
	}

	VkImageView VulkanSwapChain::_createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = NULL;
		viewInfo.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;

		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.flags = 0;

		VkImageView imageView;
		VK_CHECK_RESULT(vkCreateImageView(mDevice_, &viewInfo, nullptr, &imageView));
		return imageView;
	}

}

