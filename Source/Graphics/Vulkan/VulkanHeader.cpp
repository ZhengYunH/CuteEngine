#include "VulkanHeader.h"

VkVertexInputBindingDescription initInputBindingDesc(uint32_t binding, uint32_t stride, VkVertexInputRate rate)
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = stride;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

VkVertexInputAttributeDescription initInputAttrDesc(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
{
	VkVertexInputAttributeDescription vInputAttribDescription{};
	vInputAttribDescription.location = location;
	vInputAttribDescription.binding = binding;
	vInputAttribDescription.format = format;
	vInputAttribDescription.offset = offset;
	return vInputAttribDescription;
};

namespace Convert
{
	using zyh::ESamplerQuality;
	VkSampleCountFlagBits Convert::Quality2SamplerCount(const ESamplerQuality quality)
	{
		switch (quality)
		{
		case ESamplerQuality::None:
			return VK_SAMPLE_COUNT_1_BIT;
		case ESamplerQuality::Quality2X:
			return VK_SAMPLE_COUNT_2_BIT;
		case ESamplerQuality::Quality4X:
			return VK_SAMPLE_COUNT_4_BIT;
		case ESamplerQuality::Quality8X:
			return VK_SAMPLE_COUNT_8_BIT;
		default:
			Unimplement(0);
			break;
		}
		return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
	}

	using zyh::EPixelFormat;
	VkFormat Format(const EPixelFormat format)
	{
		switch (format)
		{
		case EPixelFormat::UNDEFINED:
			return VK_FORMAT_UNDEFINED;
		case EPixelFormat::A32R32G32B32F:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case EPixelFormat::A16B16G16R16F:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case EPixelFormat::R8G8B8A8:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case EPixelFormat::D32_SFLOAT_S8_UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		default:
			Unimplement(0);
			break;
		}
		return VK_FORMAT_MAX_ENUM;
	}
};
