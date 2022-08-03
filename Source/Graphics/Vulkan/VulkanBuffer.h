#pragma once
#include "VulkanObject.h"
#include "Math/MathUtil.h"

namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanCommandPool;

	struct VulkanBufferCollection
	{
		VkBuffer buffer;
		VkDeviceMemory mem;
	};

	class VulkanBuffer : public TVulkanObject<VulkanBufferCollection>
	{
	public:
		static void createBuffer(
			VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice,
			VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
			VkBuffer& buffer, VkDeviceMemory& bufferMemory
		);
		static void copyBuffer(VulkanCommandPool* commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	public:
		virtual ~VulkanBuffer();

		void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice);
		void setup(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		void setupData(void* data, VkDeviceSize size);
		void cleanup();

		VkDeviceSize GetBufferSize() { return mBufferSize_; }

	protected:
		void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

	protected:
		VkDeviceSize mBufferSize_;

	private:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_;
		VulkanLogicalDevice* mVulkanLogicalDevice_;
	};

	namespace UniformType
	{
		enum EUniformType
		{
			GLOBAL = 0,
			SHADER = 1,
			BATCH = 2,
			LIGHT = 3,
		};
	}
	using EUniformType = UniformType::EUniformType;
	class VulkanUniformBuffer : public VulkanBuffer
	{
	public:
		VulkanUniformBuffer(EUniformType type) 
			: VulkanBuffer()
			, mType_(type)
		{
		}

	public:
		EUniformType GetType() { return mType_; }
		void setupData(EUniformType dataType, void* data, VkDeviceSize size)
		{
			if (dataType == mType_)
				VulkanBuffer::setupData(data, size);
		}

	protected:
		EUniformType mType_;
	};

	class VulkanImageBuffer : public VulkanBuffer
	{
	public:
		VulkanImageBuffer(uint32_t width, uint32_t height, uint32_t desired_channels)
		{
			mWidth_ = width;
			mHeight_ = height;
			mChannels_ = desired_channels;
		}

		uint32_t GetWidth() { return mWidth_; }
		uint32_t GetHeight() { return mHeight_; }
		uint32_t GetChannel() { return mChannels_; }
		uint32_t GetMipLevel() { return static_cast<uint32_t>(std::floor(std::log2(Max(mWidth_, mHeight_)))) + 1; }
		uint32_t GetImageSize() { return mWidth_ * mHeight_ * mChannels_; }

	protected:
		uint32_t mWidth_;
		uint32_t mHeight_;
		uint32_t mChannels_;
	};
}