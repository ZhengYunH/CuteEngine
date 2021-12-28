#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanInstance;
	class VulkanSurface;

	enum E_QUEUE_FAMILY : uint8_t
	{
		GRAPHICS,
		PRESENT,
		COMPUTE,
		TRANSFER,
	};

	struct _QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> transferFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value() && transferFamily.has_value();
		}

		uint32_t getIndexByQueueFamily(E_QUEUE_FAMILY family)
		{
			switch (family)
			{
			case GRAPHICS:
				return graphicsFamily.value();
			case PRESENT:
				return presentFamily.value();;
			case COMPUTE:
				return computeFamily.value();;
			case TRANSFER:
				return transferFamily.value();;
			default:
				assert(0);
			}
			return 0;
		}

		bool isQueueFamilyValid(E_QUEUE_FAMILY family)
		{
			switch (family)
			{
			case GRAPHICS:
				return graphicsFamily.has_value();
			case PRESENT:
				return presentFamily.has_value();;
			case COMPUTE:
				return computeFamily.has_value();;
			case TRANSFER:
				return transferFamily.has_value();;
			default:
				assert(0);
			}
			return false;
		}
	};
	typedef TCache<_QueueFamilyIndices> QueueFamilyIndices;

	class VulkanPhysicalDevice : public TVulkanObject< VkPhysicalDevice>
	{
	public:
		void connect(VulkanInstance* instance, VulkanSurface* surface);
		void setup() override;

	private:
		VulkanInstance* mVulkanInstance_{ nullptr };
		VulkanSurface* mVulkanSurface_{ nullptr };

	public:
		const QueueFamilyIndices findQueueFamilies(const VkSurfaceKHR surface = nullptr);
		const VkPhysicalDeviceFeatures& getDeviceFeatures();
		const VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		virtual const std::vector<const char*>& getDeviceExtensions() { return mDeviceExtensions_; }
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkSampleCountFlagBits getMaxUsableSampleCount();

	protected:
		virtual VkPhysicalDevice _chooseSuitablePhysicsDevice(const std::vector<VkPhysicalDevice>& devices);
		virtual VkPhysicalDevice _chooseSuitablePhysicsDeviceExt(const std::vector<VkPhysicalDevice>& devices);

		virtual bool _isDeviceSuitable(const VkPhysicalDevice& physicalDevice);
		virtual int _rateDeviceSuitability(const VkPhysicalDevice& device);

	private:
		QueueFamilyIndices _findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface);
		bool _checkDeviceExtensionSupport(VkPhysicalDevice device);

	private: // Members
		TCache<VkPhysicalDeviceFeatures> mDeviceFeatures_{};
		QueueFamilyIndices mQueueFamilyCache_;

		const std::vector<const char*> mDeviceExtensions_ = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};


	};
}