#pragma once

#include <stdlib.h>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include <vulkan/vulkan.h>
#include "VulkanTools.h"

namespace zyh
{
	typedef struct _SwapChainBuffers {
		VkImage image;
		VkImageView view;
	} SwapChainBuffer;

	typedef struct _SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
		bool isValid{ false }; // dirty flag
	} SwapChainSupportDetails;

	class VulkanSwapChain
	{
	public:
		void initSurface(void* platformHandle, void* platformWindow);

		void connect(VkInstance instance, VkPhysicalDevice physicsDevice, VkDevice device);
		void create(uint32_t* width, uint32_t* heihgt, bool vsync = false);
		void cleanup();

		const SwapChainSupportDetails& querySwapChainSupport();
		VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
		VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

	public:
		VkFormat colorFormat;
		VkColorSpaceKHR colorSpace;
		VkSwapchainKHR swapChain = VK_NULL_HANDLE;
		uint32_t imageCount;
		std::vector<VkImage> images;
		std::vector<SwapChainBuffer> buffers;
		uint32_t queueNodeIndex = UINT32_MAX;

	protected:
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync = false);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t* width, uint32_t* height);
		VkSurfaceTransformFlagBitsKHR choosePreTransform(const VkSurfaceCapabilitiesKHR& capabilities);
		VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& capabilities);
		uint32_t getMinImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

	private:
		VkResult _createSwapChain(uint32_t* width, uint32_t* height, bool vsync = false);
		VkResult _createSwapChainImages();
		void _destroySwapChain(VkSwapchainKHR& swapChain, std::vector<SwapChainBuffer>& buffer);

		// TODO: should be helper
		VkImageView _createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	private: // members
		VkInstance	mInstance_;
		VkDevice	mDevice_;
		VkPhysicalDevice	mPhysicalDevice_;
		VkSurfaceKHR	mSurface_;
		SwapChainSupportDetails mSwapChainSupportDetails_;

	private: // Function pointers
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
		PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
		PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
		PFN_vkQueuePresentKHR fpQueuePresentKHR;
	};
};