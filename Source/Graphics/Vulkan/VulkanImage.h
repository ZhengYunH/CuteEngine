#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanCommandPool;

	struct VkImageCollection
	{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	};

	class VulkanImage : public TVulkanObject<VkImageCollection>
	{
	public:
		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice);
		virtual void setup(
			uint32_t width, uint32_t height, uint32_t mipLevels,
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags
		);

	protected:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_{ nullptr };
		VulkanLogicalDevice* mVulkanLogicalDevice_{ nullptr };

	protected:
		void _setupImage(
			uint32_t width, uint32_t height, uint32_t mipLevels,
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties
		);
		void _setupImageView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	};

	class VulkanTextureImage : public VulkanImage
	{
	public:
		VulkanTextureImage(const std::string& texturePath)
			: VulkanImage(), mTexturePath_(texturePath)
		{
		}

		void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool);

		virtual void setup(
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags
		);

	protected:
		VulkanCommandPool* mVulkanCommandPool_{ nullptr };

	protected:
		void _transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	private:
		std::string mTexturePath_;
		int mTexWidth_{ -1 };
		int mTexHeight_{ -1 };
		int mTexChannels_{ -1 };
		uint32_t mMipLevels_{ 0 };
	};
}