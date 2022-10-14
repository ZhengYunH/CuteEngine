#pragma once
#include "VulkanObject.h"
#include "VulkanImage.h"


namespace zyh
{
	class VulkanInstance;
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanSurface;
	class VulkanRenderPass;

	typedef struct _SwapChainBuffers {
		VkImage image;
		VkImageView view;
		VkFramebuffer buffer;
	} SwapChainBuffer;

	struct _SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	typedef TCache<_SwapChainSupportDetails> SwapChainSupportDetails;

	class VulkanSwapchain : public TVulkanObject<VkSwapchainKHR>
	{
	public:
		void connect(VulkanInstance* instance, VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanSurface* surface);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		void setup(uint32_t* width, uint32_t* height, bool vsync = false);
#endif
		void cleanup();
		void setupFrameBuffer(VulkanRenderPass& renderPass);

	private:
		VulkanInstance* mVulkanInstance_{ nullptr };
		VulkanPhysicalDevice* mVulkanPhysicalDevice_{ nullptr };
		VulkanLogicalDevice* mVulkanLogicalDevice_{ nullptr };
		VulkanSurface* mVulkanSurface_{ nullptr };

	public:
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		const SwapChainSupportDetails& querySwapChainSupport();
		VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
		VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
		uint32_t getImageCount();
		std::vector<VulkanImage>& getImages() { return mImages_; }
		VkFormat getColorFormat() { return mColorFormat_; }
		VkColorSpaceKHR getColorSpace() { return mColorSpace_; }
		VkExtent2D getExtend() { return mExtend2D_; };

	public:
		uint32_t queueNodeIndex = UINT32_MAX;

	protected:
		const VkSurfaceFormatKHR& getSwapSurfaceFormat();
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync = false);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t* width, uint32_t* height);
		VkSurfaceTransformFlagBitsKHR choosePreTransform(const VkSurfaceCapabilitiesKHR& capabilities);
		VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& capabilities);
		uint32_t getMinImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

	private:
		VkResult _createSwapchain(uint32_t* width, uint32_t* height, bool vsync = false);
		VkResult _createSwapchainImages();
		void _destroyOldSwapchain(VkSwapchainKHR& swapChain);

	private: // Members
		SwapChainSupportDetails mSwapChainSupportDetails_;
		VkFormat mColorFormat_;
		VkColorSpaceKHR mColorSpace_;
		VkExtent2D mExtend2D_;
		TCache<VkSurfaceFormatKHR> mSurfaceFormat_;
		TCache<uint32_t> mImageCount_;
		std::vector<VulkanImage> mImages_;
		std::vector<VulkanFrameBuffer> mBuffers_;

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