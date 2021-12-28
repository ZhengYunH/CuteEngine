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
		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool=nullptr);
		virtual void setup(
			uint32_t width, uint32_t height, uint32_t mipLevels,
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags
		);
		virtual void cleanup() override;

	protected:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_{ nullptr };
		VulkanLogicalDevice* mVulkanLogicalDevice_{ nullptr };
		VulkanCommandPool* mVulkanCommandPool_{ nullptr };

	protected:
		uint32_t mTexWidth_{ 0 };
		uint32_t mTexHeight_{ 0 };
		uint32_t mMipLevels_{ 0 };
		VkFormat mFormat_;

	protected:
		void _setupImage(
			uint32_t width, uint32_t height, uint32_t mipLevels,
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties
		);
		void _setupImageView(VkImageAspectFlags aspectFlags);

	protected: // helper function
		void _transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		void _copyBufferToImage(VkBuffer buffer);
		void _generateMipmaps();
	};

	class VulkanTextureImage : public VulkanImage
	{
	public:
		VulkanTextureImage(const std::string& texturePath)
			: VulkanImage(), mTexturePath_(texturePath)
		{
		}

		virtual void setup(
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags
		);

	protected:
		VkSampler mTextureSampler_;
		void _setupImage(VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
		void _setupSampler();

	public:
		const VkSampler& getTextureSampler() { return mTextureSampler_; }

	private:
		std::string mTexturePath_;
		int mTexChannels_{ -1 };

	};
}