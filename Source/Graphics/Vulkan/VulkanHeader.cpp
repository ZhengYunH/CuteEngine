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