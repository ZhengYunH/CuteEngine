#include "VulkanDescriptor.h"
#include "VulkanLogicalDevice.h"
#include "VulkanMaterial.h"
#include "VulkanBuffer.h"


namespace zyh
{
	void VulkanDescriptorLayout::connect(VulkanLogicalDevice* logicalDevice)
	{
		mVulkanLogicalDevice = logicalDevice;
	}

	void VulkanDescriptorLayout::setup()
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBinding;
		layoutBinding.resize(mOwner_->mUniformBuffers_.size() + mOwner_->mTextureImages_.size());

		for (auto& uniformPair : mOwner_->mUniformBuffers_)
		{
			VkDescriptorSetLayoutBinding& uboLayoutBinding = layoutBinding[uniformPair.first];
			uboLayoutBinding.binding = uniformPair.first;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = uniformPair.second[0]->GetState();
			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
		}

		for (auto& texturePair : mOwner_->mTextureImages_)
		{
			VkDescriptorSetLayoutBinding& samplerLayoutBinding = layoutBinding[texturePair.first];
			samplerLayoutBinding.binding = texturePair.first;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layoutBinding.size());
		layoutInfo.pBindings = layoutBinding.data();

		VK_CHECK_RESULT(
			vkCreateDescriptorSetLayout(mVulkanLogicalDevice->Get(), &layoutInfo, nullptr, &mVkImpl_),
			"failed to create descriptor set layout!"
		);
	}

	void VulkanDescriptorLayout::cleanup()
	{
		vkDestroyDescriptorSetLayout(mVulkanLogicalDevice->Get(), mVkImpl_, nullptr);
	}

}

