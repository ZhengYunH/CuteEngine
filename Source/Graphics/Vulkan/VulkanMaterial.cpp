#include "VulkanMaterial.h"
#include "VulkanLogicalDevice.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "Math/Matrix4x4.h"

namespace zyh
{
	VulkanMaterial::VulkanMaterial()
	{
		
	}

	void VulkanMaterial::connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, uint32_t layoutCount)
	{
		mPhysicalDevice_ = physicalDevice;
		mLogicalDevice_ = logicalDevice;
		mLayoutCount_ = layoutCount;
		mGraphicsPipeline_->connect(logicalDevice);
	}

	void VulkanMaterial::setup()
	{
		createGraphicsPipeline();
		createUniformBuffers();
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
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		for (auto& uniformBuffer : mUniformBuffers_)
		{
			uniformBuffer = new VulkanBuffer();
			uniformBuffer->connect(mPhysicalDevice_, mLogicalDevice_);
			uniformBuffer->setup(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
	}

	void VulkanMaterial::createDesciptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(mLayoutCount_);

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(mLayoutCount_);

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

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

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

			vkUpdateDescriptorSets(mLogicalDevice_->Get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanMaterial::cleanup()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	void VulkanMaterial::updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};

		auto convertToGlmMat = [](const Matrix4x4& mat) -> glm::mat4x4
		{
			return glm::mat4x4
			{
				mat.m00, mat.m01, mat.m02, mat.m03,
				mat.m10, mat.m11, mat.m12, mat.m13,
				mat.m20, mat.m21, mat.m22, mat.m23,
				mat.m30, mat.m31, mat.m32, mat.m33,
			};
		};

		Matrix4x3 modelMat = Matrix4x3();
		modelMat.SetRotationX(DegreeToRadian(-90.f), Vector3::GetZero());
		Matrix4x3 viewMat = mCamera_.getViewMatrix();
		Matrix4x4 projMat = mCamera_.getProjMatrix();

		ubo.model = convertToGlmMat(modelMat);
		ubo.view = convertToGlmMat(viewMat);
		ubo.proj = convertToGlmMat(projMat);
		ubo.proj[1][1] *= -1;

		mUniformBuffers_[currentImage]->setupData(&ubo, sizeof(ubo));
	}

}
