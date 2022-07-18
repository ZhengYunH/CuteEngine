#include "Core/Engine.h"
#include "Core/ClientScene.h"
#include "Camera/Camera.h"

#include "VulkanBase.h"
#include "VulkanInstance.h"
#include "VulkanMaterial.h"
#include "VulkanLogicalDevice.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "Math/Matrix4x4.h"
#include "VulkanRenderPass.h"

namespace zyh
{
	VulkanMaterial::VulkanMaterial()
	{
		mRenderPass_ = new VulkanRenderPassBase("Resource/shaders/vert.spv", "Resource/shaders/frag.spv");
		mGraphicsPipeline_ = new VulkanGraphicsPipeline();
	}

	void VulkanMaterial::connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, uint32_t layoutCount)
	{
		mPhysicalDevice_ = physicalDevice;
		mLogicalDevice_ = logicalDevice;
		mLayoutCount_ = layoutCount;
		mRenderPass_->connect(mLogicalDevice_);
		mGraphicsPipeline_->connect(mLogicalDevice_, mRenderPass_);
	}

	void VulkanMaterial::setup()
	{
		mRenderPass_->setup(*GInstance->mColorFormat_, *GInstance->mMsaaSamples_, *GInstance->mDepthFormat_);
		createGraphicsPipeline();
		createUniformBuffers();
		createTextureImage();
		createDesciptorPool();
		createDescriptorSets();
	}

	void VulkanMaterial::createGraphicsPipeline()
	{
		mGraphicsPipeline_->setup();
	}

	void VulkanMaterial::createUniformBuffers()
	{
		mUniformBuffers_.resize(mLayoutCount_);
		mUniformLightBuffers_.resize(mLayoutCount_);
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		for (auto& uniformBuffer : mUniformBuffers_)
		{
			uniformBuffer = new VulkanBuffer();
			uniformBuffer->connect(mPhysicalDevice_, mLogicalDevice_);
			uniformBuffer->setup(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		bufferSize = sizeof(UniformLightingBufferObject);
		for (auto& uniformBuffer : mUniformLightBuffers_)
		{
			uniformBuffer = new VulkanBuffer();
			uniformBuffer->connect(mPhysicalDevice_, mLogicalDevice_);
			uniformBuffer->setup(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
	}

	void VulkanMaterial::createDesciptorPool()
	{
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(mLayoutCount_);

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(mLayoutCount_);

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(mLayoutCount_);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(mLayoutCount_);

		VK_CHECK_RESULT(vkCreateDescriptorPool(mLogicalDevice_->Get(), &poolInfo, nullptr, &mDescriptorPool_), "failed to create descriptor pool!");
	}

	void VulkanMaterial::createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(mLayoutCount_, mGraphicsPipeline_->getDescriptorSetLayout());

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool_;
		allocInfo.descriptorSetCount = mLayoutCount_;
		allocInfo.pSetLayouts = layouts.data();

		mDescriptorSets_.resize(mLayoutCount_);
		VK_CHECK_RESULT(vkAllocateDescriptorSets(mLogicalDevice_->Get(), &allocInfo, mDescriptorSets_.data()), "failed to allocate descriptor sets!");

		for (size_t i = 0; i < mDescriptorSets_.size(); i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers_[i]->Get().buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = mTextureImage_->Get().view;
			imageInfo.sampler = mTextureImage_->getTextureSampler();

			VkDescriptorBufferInfo lightingbufferInfo{};
			lightingbufferInfo.buffer = mUniformLightBuffers_[i]->Get().buffer;
			lightingbufferInfo.offset = 0;
			lightingbufferInfo.range = sizeof(UniformLightingBufferObject);

			std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = mDescriptorSets_[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = mDescriptorSets_[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = mDescriptorSets_[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pBufferInfo = &lightingbufferInfo;

			vkUpdateDescriptorSets(mLogicalDevice_->Get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanMaterial::createTextureImage()
	{
		mTextureImage_ = new VulkanTextureImage("Resource/textures/viking_room.png");
		mTextureImage_->connect(mPhysicalDevice_, mLogicalDevice_, GVulkanInstance->mGraphicsCommandPool_);
		mTextureImage_->setup(VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	void VulkanMaterial::cleanup()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	void VulkanMaterial::updateUniformBuffer(UniformBufferObject& ubo, UniformLightingBufferObject& ulbo)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		ulbo.directionalLight = DirectionLight
		(
			Vector3(0.5, 0.5, 1.0),
			Vector3(0.1, 0.1, 0.1),
			Vector3(0.5, 0.5, 0.5),
			Vector3(0.2, 0.2, 0.2)
		);
		ulbo.numOfPointLights = 1;
		ulbo.pointLights[0] = PointLight
		(
			Vector3(-0.5, -0.5, 1.0),
			Vector3(0.1, 0.1, 0.1),
			Vector3(0.3, 0.3, 0.3),
			Vector3(0.3, 0.3, 0.3)
		);
		ulbo.spotLight = SpotLight
		(
			Vector3(0.5, 0.5, 1.0),
			Vector3(-0.5, -0.5, -1.0),
			Vector3(0.3, 0.3, 0.3),
			Vector3(0.5, 0.5, 0.5),
			Vector3(0.3, 0.3, 0.3)
		);
	}

	void VulkanMaterial::endUpdateUniformBuffer(UniformBufferObject& ubo, UniformLightingBufferObject& ulbo)
	{
		mUniformBuffers_[mCurrentUpdateImage_]->setupData(&ubo, sizeof(ubo));
		mUniformLightBuffers_[mCurrentUpdateImage_]->setupData(&ulbo, sizeof(ulbo));
	}

	VkPipelineLayout VulkanMaterial::getPipelineLayout()
	{
		return mGraphicsPipeline_->getPipelineLayout();
	}


}
