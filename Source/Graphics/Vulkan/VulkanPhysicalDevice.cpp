#include "VulkanPhysicalDevice.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"

namespace zyh
{
	void VulkanPhysicalDevice::connect(VulkanInstance* instance, VulkanSurface* surface)
	{
		mVulkanInstance_ = instance;
		mVulkanSurface_ = surface;
	}

	void VulkanPhysicalDevice::setup()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(*mVulkanInstance_, &deviceCount, nullptr);
		HYBRID_CHECK(deviceCount != 0, "failed to find GPUs with Vulkan support!");
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(*mVulkanInstance_, &deviceCount, devices.data());

		mVkImpl_ = _chooseSuitablePhysicsDevice(devices);
		HYBRID_CHECK(mVkImpl_, "failed to find a suitable GPU!");

		*GInstance->mMsaaSamples_ = getMaxUsableSampleCount();
		GInstance->mMsaaSamples_.IsValid(true);

		*GInstance->mDepthFormat_ = findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
		GInstance->mDepthFormat_.IsValid(true);

	}

	const QueueFamilyIndices VulkanPhysicalDevice::findQueueFamilies(const VkSurfaceKHR surface /*= nullptr*/)
	{
		if (!surface)
		{
			if (!mQueueFamilyCache_.IsValid())
			{
				mQueueFamilyCache_ = _findQueueFamilies(mVkImpl_, mVulkanSurface_->Get());
			}
			return mQueueFamilyCache_;
		}
		else
		{
			return _findQueueFamilies(mVkImpl_, surface);
		}
	}

	const VkPhysicalDeviceFeatures& VulkanPhysicalDevice::getDeviceFeatures()
	{
		if (!mDeviceFeatures_.IsValid())
		{
			mDeviceFeatures_->samplerAnisotropy = VK_TRUE;
			mDeviceFeatures_->sampleRateShading = VK_TRUE; // enable sample shading feature for the device
		}
		return *mDeviceFeatures_;
	}

	const VkFormat VulkanPhysicalDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		HYBRID_CHECK(mVkImpl_);

		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(mVkImpl_, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		tools::exitFatal("failed to find supported format!");
		return candidates[0];
	}

	uint32_t VulkanPhysicalDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		HYBRID_CHECK(mVkImpl_);

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(mVkImpl_, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		tools::exitFatal("failed to find suitable memory type!");
		return 0;
	}

	VkSampleCountFlagBits VulkanPhysicalDevice::getMaxUsableSampleCount()
	{
		HYBRID_CHECK(mVkImpl_);

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(mVkImpl_, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	VkPhysicalDevice VulkanPhysicalDevice::_chooseSuitablePhysicsDevice(const std::vector<VkPhysicalDevice>& devices)
	{
		for (const auto& device : devices) {
			if (_isDeviceSuitable(device)) 
			{
				return device;
			}
		}
		tools::exitFatal("failed to find suitable physics device!");
		return devices[0];
	}

	VkPhysicalDevice VulkanPhysicalDevice::_chooseSuitablePhysicsDeviceExt(const std::vector<VkPhysicalDevice>& devices)
	{
		// Use an ordered map to automatically sort candidates by increasing score
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : devices) {
			int score = _rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}
		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0) {
			return candidates.rbegin()->second;
		}
		return VK_NULL_HANDLE;
	}

	bool VulkanPhysicalDevice::_isDeviceSuitable(const VkPhysicalDevice& physicalDevice)
	{
		bool extensionsSupported = _checkDeviceExtensionSupport(physicalDevice);
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
		return extensionsSupported && supportedFeatures.samplerAnisotropy;
	}

	int VulkanPhysicalDevice::_rateDeviceSuitability(const VkPhysicalDevice& device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		int score = 0;

		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		// Maximum possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		// Application can't function without geometry shaders
		if (!deviceFeatures.geometryShader) {
			return 0;
		}

		return score;
	}

	QueueFamilyIndices VulkanPhysicalDevice::_findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

		int i = 0;
		for (const auto& queueFamilyPropertie : queueFamilyProperties) {
			// Try to find a queue family index that supports compute but not graphics
			if ((queueFamilyPropertie.queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyPropertie.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				indices->computeFamily = i;
				continue;
			}

			// Try to find a queue family index that supports transfer but not graphics and compute
			if ((queueFamilyPropertie.queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyPropertie.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyPropertie.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				indices->transferFamily = i;
				continue;
			}

			if (queueFamilyPropertie.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices->graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				indices->presentFamily = i;

			if (indices->isComplete()) {
				break;
			}

			++i;
		}

		indices.IsValid(true);
		return indices;
	}

	bool VulkanPhysicalDevice::_checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		const std::vector<const char*>& deviceExntension = getDeviceExtensions();
		std::set<std::string> requiredExtensions(deviceExntension.begin(), deviceExntension.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

}

