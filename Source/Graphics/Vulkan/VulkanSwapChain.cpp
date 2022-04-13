#include "VulkanSwapchain.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSurface.h"
#include "VulkanRenderPass.h"


namespace zyh
{
	void VulkanSwapchain::connect(VulkanInstance* instance, VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanSurface* surface)
	{
		mVulkanInstance_ = instance;
		mVulkanPhysicalDevice_ = physicalDevice;
		mVulkanLogicalDevice_ = logicalDevice;
		mVulkanSurface_ = surface;
	}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	void VulkanSwapchain::setup(uint32_t* width, uint32_t* height, bool vsync)
	{
		auto vkInstance = mVulkanInstance_->Get();
		auto vkDevice = mVulkanLogicalDevice_->Get();

		fpGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
		fpGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
		fpGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
		fpGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

		fpCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(vkDevice, "vkCreateSwapchainKHR"));
		fpDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(vkDevice, "vkDestroySwapchainKHR"));
		fpGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(vkDevice, "vkGetSwapchainImagesKHR"));
		fpAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(vkDevice, "vkAcquireNextImageKHR"));
		fpQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(vkDevice, "vkQueuePresentKHR"));

		VkSwapchainKHR oldSwapchain = mVkImpl_;
		VK_CHECK_RESULT(_createSwapchain(width, height, vsync));
		_destroyOldSwapchain(oldSwapchain, mBuffers_);
		VK_CHECK_RESULT(_createSwapchainImages());

		// set attribute
		QueueFamilyIndices indice = mVulkanPhysicalDevice_->findQueueFamilies();
		if (!indice->graphicsFamily || !indice->presentFamily)
		{
			tools::exitFatal("Could not find a graphics and/or presenting queue!");
		}

		const VkSurfaceFormatKHR& surfaceFormat = getSwapSurfaceFormat();
		mColorFormat_ = surfaceFormat.format;
		mColorSpace_ = surfaceFormat.colorSpace;

		*GInstance->mColorFormat_ = mColorFormat_;
		GInstance->mColorFormat_.IsValid(true);

		*GInstance->mExtend_ = mExtend2D_;
		GInstance->mExtend_.IsValid(true);
	}
#endif 

	void VulkanSwapchain::cleanup()
	{
		_destroyOldSwapchain(mVkImpl_, mBuffers_);
		mSwapChainSupportDetails_.IsValid(false);
		mVkImpl_ = VK_NULL_HANDLE;
	}

	void VulkanSwapchain::setupFrameBuffer(VulkanRenderPassBase& renderPass, std::vector<VkImageView>& attachments)
	{
		for (auto& buffer : mBuffers_)
		{
			std::vector<VkImageView> fullAttachments(attachments.begin(), attachments.end());
			fullAttachments.push_back(buffer.view);

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass.Get();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(fullAttachments.size());;
			framebufferInfo.pAttachments = fullAttachments.data();
			framebufferInfo.width = mExtend2D_.width;
			framebufferInfo.height = mExtend2D_.height;
			framebufferInfo.layers = 1;

			VK_CHECK_RESULT(vkCreateFramebuffer(mVulkanLogicalDevice_->Get(), &framebufferInfo, nullptr, &buffer.buffer), "failed to create framebuffer!");
		}
	}

	SwapChainSupportDetails VulkanSwapchain::querySwapChainSupport(VkPhysicalDevice device)
	{
		HYBRID_CHECK(device);
		HYBRID_CHECK(mVulkanSurface_);

		SwapChainSupportDetails& details = mSwapChainSupportDetails_;

		// simply query capabilities
		fpGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mVulkanSurface_->Get(), &details->capabilities);

		// query the supported surface formats
		uint32_t formatCount;
		fpGetPhysicalDeviceSurfaceFormatsKHR(device, mVulkanSurface_->Get(), &formatCount, nullptr);
		if (formatCount > 0) {
			details->formats.resize(formatCount);
			fpGetPhysicalDeviceSurfaceFormatsKHR(device, mVulkanSurface_->Get(), &formatCount, details->formats.data());
		}

		// query the supported presentation modes
		uint32_t presentModeCount;
		fpGetPhysicalDeviceSurfacePresentModesKHR(device, mVulkanSurface_->Get(), &presentModeCount, nullptr);
		if (presentModeCount > 0) {
			details->presentModes.resize(presentModeCount);
			fpGetPhysicalDeviceSurfacePresentModesKHR(device, mVulkanSurface_->Get(), &presentModeCount, details->presentModes.data());
		}
		details.IsValid(true);

		return details;
	}

	const SwapChainSupportDetails& VulkanSwapchain::querySwapChainSupport()
	{
		if (!mSwapChainSupportDetails_.IsValid())
		{
			mSwapChainSupportDetails_ = querySwapChainSupport(mVulkanPhysicalDevice_->Get());
		}
		return mSwapChainSupportDetails_;
	}

	VkResult VulkanSwapchain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex)
	{
		return fpAcquireNextImageKHR(mVulkanLogicalDevice_->Get(), mVkImpl_, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, imageIndex);
	}

	VkResult VulkanSwapchain::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore /*= VK_NULL_HANDLE*/)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &mVkImpl_;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;
		}
		return fpQueuePresentKHR(queue, &presentInfo);
	}

	uint32_t VulkanSwapchain::getImageCount()
	{
		if (!mImageCount_.IsValid())
		{
			HYBRID_CHECK(mVulkanLogicalDevice_);
			VK_CHECK_RESULT(fpGetSwapchainImagesKHR(mVulkanLogicalDevice_->Get(), mVkImpl_, &(*mImageCount_), NULL));
			mImageCount_.IsValid(true);
		}
		return *mImageCount_;
	}

	VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		HYBRID_CHECK(availableFormats.size() > 0);

		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			return VkSurfaceFormatKHR{
				VK_FORMAT_B8G8R8A8_UNORM,
				availableFormats[0].colorSpace
			};
		}

		// iterate over the list of available surface format and
		// check for the presence of VK_FORMAT_B8G8R8A8_UNORM // VK_FORMAT_B8G8R8_SRGB
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}
		
		return availableFormats[0];
	}

	const VkSurfaceFormatKHR& VulkanSwapchain::getSwapSurfaceFormat()
	{
		if (!mSurfaceFormat_.IsValid())
		{
			const SwapChainSupportDetails& swapChainSupport = querySwapChainSupport();
			*mSurfaceFormat_ = chooseSwapSurfaceFormat(swapChainSupport->formats);
			mSurfaceFormat_.IsValid(true);
		}
		return *mSurfaceFormat_;
	}

	VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync /*= false*/)
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

	VkExtent2D VulkanSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t* width, uint32_t* height)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			*width = capabilities.currentExtent.width;
			*height = capabilities.currentExtent.height;
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = { *width, *height };

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	VkSurfaceTransformFlagBitsKHR VulkanSwapchain::choosePreTransform(const VkSurfaceCapabilitiesKHR& capabilities)
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

	VkCompositeAlphaFlagBitsKHR VulkanSwapchain::chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& capabilities)
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

	uint32_t VulkanSwapchain::getMinImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		uint32_t desiredNumberOfSwapchainImages = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && desiredNumberOfSwapchainImages > capabilities.maxImageCount) {
			desiredNumberOfSwapchainImages = capabilities.maxImageCount;
		}
		return desiredNumberOfSwapchainImages;
	}

	VkResult VulkanSwapchain::_createSwapchain(uint32_t* width, uint32_t* height, bool vsync /*= false*/)
	{
		VkSwapchainKHR oldSwapchain = mVkImpl_;

		const SwapChainSupportDetails& swapChainSupport = querySwapChainSupport();
		const VkSurfaceCapabilitiesKHR& surfCaps = swapChainSupport->capabilities;
		const VkSurfaceFormatKHR& surfaceFormat = getSwapSurfaceFormat();
		mExtend2D_ = chooseSwapExtent(surfCaps, width, height);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mVulkanSurface_->Get();
		createInfo.minImageCount = getMinImageCount(surfCaps);
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = mExtend2D_;
		createInfo.imageArrayLayers = 1;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)choosePreTransform(surfCaps);
		// Setting oldSwapChain to the saved handle of the previous mVkInstance_ aids in resource reuse and makes sure that we can still present already acquired images
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

		return fpCreateSwapchainKHR(mVulkanLogicalDevice_->Get(), &createInfo, nullptr, &mVkImpl_);
	}
	

	VkResult VulkanSwapchain::_createSwapchainImages()
	{
		uint32_t imageCount = getImageCount();
		*GInstance->mImageCount_ = imageCount;
		GInstance->mImageCount_.IsValid(true);

		mImages_.resize(imageCount);
		VK_CHECK_RESULT(fpGetSwapchainImagesKHR(mVulkanLogicalDevice_->Get(), mVkImpl_, &imageCount, mImages_.data()));

		const VkSurfaceFormatKHR& surfaceFormat = getSwapSurfaceFormat();

		mBuffers_.resize(imageCount);
		for (size_t i = 0; i < imageCount; i++) {
			mBuffers_[i].image = mImages_[i];
			mBuffers_[i].view = _createImageView(mBuffers_[i].image, surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
		return VK_SUCCESS;
	}

	void VulkanSwapchain::_destroyOldSwapchain(VkSwapchainKHR& swapchain, std::vector<SwapChainBuffer>& buffers)
	{
		if (swapchain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < mBuffers_.size(); ++i)
			{
				vkDestroyImageView(mVulkanLogicalDevice_->Get(), mBuffers_[i].view, nullptr);
			}
			fpDestroySwapchainKHR(mVulkanLogicalDevice_->Get(), swapchain, nullptr);
		}
	}

	VkImageView VulkanSwapchain::_createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;

		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		VkImageView imageView;
		VK_CHECK_RESULT (vkCreateImageView(mVulkanLogicalDevice_->Get(), &viewInfo, nullptr, &imageView), "failed to create texture image view!");

		return imageView;
	}
}

