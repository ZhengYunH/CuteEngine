#include "VulkanImage.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanBuffer.h"

#include <stb_image.h>


namespace zyh
{
	void VulkanImage::connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool)
	{
		mVulkanPhysicalDevice_ = physicalDevice;
		mVulkanLogicalDevice_ = logicalDevice;
		mVulkanCommandPool_ = commandPool;
	}

	void VulkanImage::setup(
		uint32_t width, uint32_t height, uint32_t mipLevels,
		VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags
	)
	{
		_setupImage(width, height, mipLevels, numSamples, format, tiling, usage, properties);
		_setupImageView(aspectFlags);
	}

	void VulkanImage::cleanup()
	{
		vkDestroyImageView(mVulkanLogicalDevice_->Get(), mVkImpl_.view, nullptr);
		vkDestroyImage(mVulkanLogicalDevice_->Get(), mVkImpl_.image, nullptr);
		vkFreeMemory(mVulkanLogicalDevice_->Get(), mVkImpl_.mem, nullptr);
		
		mVkImpl_.view = VK_NULL_HANDLE;
		mVkImpl_.image = VK_NULL_HANDLE;
		mVkImpl_.mem = VK_NULL_HANDLE;
	}

	void VulkanImage::_setupImage(
		uint32_t width, uint32_t height, uint32_t mipLevels,
		VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties
	)
	{
		// save properties
		mTexWidth_ = width;
		mTexHeight_ = height;
		mMipLevels_ = mipLevels;
		mFormat_ = format;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.samples = numSamples;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = 0; // Optional

		if (vkCreateImage(mVulkanLogicalDevice_->Get(), &imageInfo, nullptr, &mVkImpl_.image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(mVulkanLogicalDevice_->Get(), mVkImpl_.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = mVulkanPhysicalDevice_->findMemoryType(memRequirements.memoryTypeBits, properties);

		VK_CHECK_RESULT(vkAllocateMemory(mVulkanLogicalDevice_->Get(), &allocInfo, nullptr, &mVkImpl_.mem), "failed to allocate image memory!");

		vkBindImageMemory(mVulkanLogicalDevice_->Get(), mVkImpl_.image, mVkImpl_.mem, 0);
	}

	void VulkanImage::_setupImageView(VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = mVkImpl_.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = mFormat_;

		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mMipLevels_;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VK_CHECK_RESULT(vkCreateImageView(mVulkanLogicalDevice_->Get(), &viewInfo, nullptr, &mVkImpl_.view), "failed to create texture image view!");
	}

	void VulkanImage::_transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		HYBRID_CHECK(mVulkanLogicalDevice_);
		SingleTimeExecFunc func = [&](VulkanCommand& commandBuffer)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.image = mVkImpl_.image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = mMipLevels_;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			barrier.srcAccessMask = 0; // TODO
			barrier.dstAccessMask = 0; // TODO

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else {
				throw std::invalid_argument("unsupported layout transition!");
			}

			vkCmdPipelineBarrier(
				commandBuffer.Get(),
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		};

		mVulkanCommandPool_->generateSingleTimeCommand(func);
	}

	void VulkanImage::_copyBufferToImage(VkBuffer buffer)
	{
		HYBRID_CHECK(mVulkanLogicalDevice_);
		SingleTimeExecFunc func = [&](VulkanCommand& commandBuffer)
		{
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				mTexWidth_,
				mTexHeight_,
				1
			};

			vkCmdCopyBufferToImage(
				commandBuffer,
				buffer,
				mVkImpl_.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);
		};

		mVulkanCommandPool_->generateSingleTimeCommand(func);
	}

	void VulkanImage::_generateMipmaps()
	{
		HYBRID_CHECK(mVulkanPhysicalDevice_);
		HYBRID_CHECK(mVulkanCommandPool_);

		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(mVulkanPhysicalDevice_->Get(), mFormat_, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		SingleTimeExecFunc func = [&](VulkanCommand& commandBuffer)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = mVkImpl_.image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

			int32_t mipWidth = mTexWidth_;
			int32_t mipHeight = mTexHeight_;

			for (uint32_t i = 1; i < mMipLevels_; i++) {
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer.Get(),
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				vkCmdBlitImage(commandBuffer.Get(),
					mVkImpl_.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					mVkImpl_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit,
					VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer.Get(),
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				if (mipWidth > 1) mipWidth /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mMipLevels_ - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer.Get(),
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		};

		mVulkanCommandPool_->generateSingleTimeCommand(func);
	}

	void VulkanTextureImage::setup(
		VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags
	)
	{
		HYBRID_CHECK(mVulkanCommandPool_);
		_setupImage(numSamples, format, tiling, usage, properties);
		_setupImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		_setupSampler();
	}

	
	void VulkanTextureImage::_setupImage(VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		int width, height, channels, mipLevel;

		stbi_uc* pixels = stbi_load(mTexturePath_.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		VkDeviceSize imageSize = width * height * 4;
		mipLevel = static_cast<uint32_t>(std::floor(std::log2(max(width, height)))) + 1;
		HYBRID_CHECK(pixels);

		VulkanBuffer buffer;
		buffer.connect(mVulkanPhysicalDevice_, mVulkanLogicalDevice_);
		buffer.setup(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		buffer.setupData(pixels, imageSize);
		stbi_image_free(pixels);

		VulkanImage::_setupImage(
			static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(mipLevel),
			numSamples, format, tiling, usage, properties
		);

		_transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		_copyBufferToImage(buffer.Get().buffer);
		_generateMipmaps();
	}

	void VulkanTextureImage::_setupSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(mVulkanPhysicalDevice_->Get(), &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(mMipLevels_);

		VK_CHECK_RESULT(vkCreateSampler(mVulkanLogicalDevice_->Get(), &samplerInfo, nullptr, &mTextureSampler_), "failed to create texture sampler!");
	}

}

