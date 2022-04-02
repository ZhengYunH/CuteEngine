#include "VulkanPrimitives.h"
#include "VulkanMaterial.h"

namespace zyh
{

	void VulkanPrimitives::draw(VkCommandBuffer commandBuffer)
	{
		VkBuffer vertexBuffers[] = { mVertexBuffer_->Get().buffer };
		VkDeviceSize offsets[] = { 0 };
		VulkanMaterial* material = dynamic_cast<VulkanMaterial*>(mMaterial_);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer_->Get().buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->getPipelineLayout(), 0, 1, material->getDescriptorSet(), 0, nullptr);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mIndices_.size()), 1, 0, 0, 0);
	}
}