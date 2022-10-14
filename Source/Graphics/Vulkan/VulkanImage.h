#pragma once
#include "VulkanObject.h"
#include "Graphics/Common/RenderResource.h"

namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanCommandPool;
	class VulkanBuffer;
	class VulkanImageBuffer;
	class VulkanRenderPass;

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

	public:
		void createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		
		uint32_t GetWidth() { return mTexWidth_; }
		uint32_t GetHeight() { return mTexHeight_; }
		uint32_t GetMipLevels() { return mMipLevels_; }
		VkFormat GetFormat() { return mFormat_; }

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
		virtual void _setupImage(
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


	class VulkanFrameBuffer : public TVulkanObject<VkFramebuffer>
	{
	public:
		VulkanFrameBuffer()
		{

		}
		~VulkanFrameBuffer()
		{
			cleanup();
		}

		void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice)
		{
			mVulkanPhysicalDevice_ = physicalDevice;
			mVulkanLogicalDevice_ = logicalDevice;
		}
		void createResource(VulkanRenderPass& renderPass);
		void setup(VulkanRenderPass& renderPass, VkImageView* extraViews = nullptr, size_t extraViewCount = 0);
		void setup(VulkanRenderPass& renderPass, std::vector<VkImageView> views);
		void cleanup();


	protected:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_{ nullptr };
		VulkanLogicalDevice* mVulkanLogicalDevice_{ nullptr };

	protected:
		std::vector<VulkanImage*> mResources_;
	};


	class VulkanTexture : public VulkanImage
	{
	public:
		virtual const VkSampler& getTextureSampler() = 0;
	protected:
		virtual VulkanBuffer* _getImageBuffer() = 0;
	};

	class VulkanTextureImage : public VulkanTexture
	{
	public:
		VulkanTextureImage(const std::string& texturePath)
			: VulkanTexture(), mTexturePath_(texturePath)
		{
		}

		void setup(
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags
		);
	protected:
		VkSampler mTextureSampler_ {VK_NULL_HANDLE};
		void _setupImage(
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties
		);
		void _setupSampler();
		virtual VulkanBuffer* _getImageBuffer() override;

	public:
		virtual const VkSampler& getTextureSampler() override { return mTextureSampler_; }

	private:
		std::string mTexturePath_;
		int mTexChannels_{ -1 };
	};

	class VulkanRawlImage : public VulkanTexture
	{
	public:
		VulkanRawlImage(unsigned char* data) 
			: VulkanTexture()
			, mData_(data)
		{
		}
		virtual void setup(
			uint32_t width, uint32_t height, uint32_t mipLevels,
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags
		);

	protected:
		void _setupSampler();
		virtual void _setupImage(
			uint32_t width, uint32_t height, uint32_t channel,
			VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties
		) override;
		virtual VulkanBuffer* _getImageBuffer() override;

	public:
		virtual const VkSampler& getTextureSampler() override { return mTextureSampler_; }

	protected:
		VkSampler mTextureSampler_{ VK_NULL_HANDLE };

		unsigned char* mData_;
		uint32_t mWidth_;
		uint32_t mHeight_; 
		uint32_t mChannels_;

	};
}