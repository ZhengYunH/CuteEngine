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
#include "Graphics/Vulkan/VulkanSwapchain.h"

#include "Graphics/Imgui/imgui.h"
#include "Graphics/Imgui/imgui_impl_vulkan.h"
#include "Graphics/Imgui/imgui_impl_win32.h"


namespace zyh
{
	VulkanMaterial::VulkanMaterial(IMaterial* material, VulkanRenderPassBase::OpType opType)
	{
		mMaterial_ = material;
		mRenderPass_ = new VulkanRenderPassBase(opType);
		mGraphicsPipeline_ = new VulkanGraphicsPipeline(this);
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
		createUniformBuffers();
		createTextureImages();
		createGraphicsPipeline();
		createDesciptorPool();
		createDescriptorSets();
	}

	void VulkanMaterial::createGraphicsPipeline()
	{
		mGraphicsPipeline_->setup();
	}

	void VulkanMaterial::createUniformBuffers()
	{
		std::vector<VulkanUniformBuffer*> uniformBuffers;
		uniformBuffers.resize(mLayoutCount_); 

		std::vector<VulkanUniformBuffer*> lightBuffers;
		lightBuffers.resize(mLayoutCount_);

		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		for (auto& uniformBuffer : uniformBuffers)
		{
			uniformBuffer = new VulkanUniformBuffer(EUniformType::BATCH, VK_SHADER_STAGE_VERTEX_BIT);
			uniformBuffer->connect(mPhysicalDevice_, mLogicalDevice_);
			uniformBuffer->setup(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
		
		bufferSize = sizeof(UniformLightingBufferObject);
		for (auto& uniformBuffer : lightBuffers)
		{
			uniformBuffer = new VulkanUniformBuffer(EUniformType::LIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
			uniformBuffer->connect(mPhysicalDevice_, mLogicalDevice_);
			uniformBuffer->setup(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		mUniformBuffers_[0] = std::move(uniformBuffers);
		mUniformBuffers_[2] = std::move(lightBuffers);

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
			DEBUG_RUN(std::set<uint32_t> debugBindingSet;)
			std::vector<VkWriteDescriptorSet> descriptorWrites;
			descriptorWrites.resize(mUniformBuffers_.size() + mTextureImages_.size());
			VkWriteDescriptorSet writeInfo{};
			for (auto& uniformPair : mUniformBuffers_)
			{
				uint32_t binding = uniformPair.first;
				VulkanBuffer* vulkanBuffer = uniformPair.second[i];
				DEBUG_RUN(HYBRID_CHECK(debugBindingSet.find(binding) == debugBindingSet.end());)

				// VkWriteDescriptorSet& writeInfo = descriptorWrites[binding];

				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = vulkanBuffer->Get().buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = vulkanBuffer->GetBufferSize();

				writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeInfo.dstSet = mDescriptorSets_[i];
				writeInfo.dstBinding = binding;
				writeInfo.dstArrayElement = 0;
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeInfo.descriptorCount = 1;
				writeInfo.pBufferInfo = &bufferInfo;
				vkUpdateDescriptorSets(mLogicalDevice_->Get(), 1, &writeInfo, 0, nullptr);
				DEBUG_RUN(debugBindingSet.insert(binding);)
			}

			for (auto& texturePair : mTextureImages_)
			{
				uint32_t binding = texturePair.first;
				VulkanTextureImage* image = static_cast<VulkanTextureImage*>(texturePair.second);
				DEBUG_RUN(HYBRID_CHECK(debugBindingSet.find(binding) == debugBindingSet.end());)

				// VkWriteDescriptorSet& writeInfo = descriptorWrites[binding];

				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = image->Get().view;
				imageInfo.sampler = image->getTextureSampler();

				writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeInfo.dstSet = mDescriptorSets_[i];
				writeInfo.dstBinding = binding;
				writeInfo.dstArrayElement = 0;
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeInfo.descriptorCount = 1;
				writeInfo.pImageInfo = &imageInfo;
				vkUpdateDescriptorSets(mLogicalDevice_->Get(), 1, &writeInfo, 0, nullptr);
				DEBUG_RUN(debugBindingSet.insert(binding);)
			}
			// vkUpdateDescriptorSets(mLogicalDevice_->Get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanMaterial::createTextureImages()
	{
		VulkanTextureImage* baseTextureImage_ = new VulkanTextureImage("Resource/textures/viking_room.png");
		baseTextureImage_->connect(mPhysicalDevice_, mLogicalDevice_, GVulkanInstance->mGraphicsCommandPool_);
		baseTextureImage_->setup(VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
		);

		mTextureImages_[1] = baseTextureImage_;
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
	}

	void VulkanMaterial::endUpdateUniformBuffer(UniformBufferObject& ubo, UniformLightingBufferObject& ulbo)
	{
		for (auto& uniformPair : mUniformBuffers_)
		{
			uniformPair.second[mCurrentUpdateImage_]->setupData(EUniformType::BATCH, &ubo, sizeof(ubo));
			uniformPair.second[mCurrentUpdateImage_]->setupData(EUniformType::LIGHT, &ulbo, sizeof(ulbo));
		}
	}

	VkPipelineLayout VulkanMaterial::getPipelineLayout()
	{
		return mGraphicsPipeline_->getPipelineLayout();
	}

	VkPipeline VulkanMaterial::getPipeline()
	{
		return mGraphicsPipeline_->Get();
	}

	void ImGuiMaterial::createTextureImages()
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = (float)GVulkanInstance->mSwapchain_->getExtend().width;
		io.DisplaySize.y = (float)GVulkanInstance->mSwapchain_->getExtend().height;

		int width, height, channel;
		unsigned char* pixels = NULL;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &channel);

		VulkanRawlImage* baseTextureImage_ = new VulkanRawlImage(pixels);
		baseTextureImage_->connect(mPhysicalDevice_, mLogicalDevice_, GVulkanInstance->mGraphicsCommandPool_);
		baseTextureImage_->setup(width, height, channel,
			VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
		);

		mTextureImages_[0] = baseTextureImage_;
	}

	void ImGuiMaterial::getBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(ImDrawVert);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		descriptions.push_back(std::move(bindingDescription));
	}

	void ImGuiMaterial::getAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
	{
		auto generateDesc = [](
			uint32_t binding,
			uint32_t location,
			VkFormat format,
			uint32_t offset)->VkVertexInputAttributeDescription
		{
			VkVertexInputAttributeDescription vInputAttribDescription{};
			vInputAttribDescription.location = location;
			vInputAttribDescription.binding = binding;
			vInputAttribDescription.format = format;
			vInputAttribDescription.offset = offset;
			return vInputAttribDescription;
		};

		descriptions = {
			generateDesc(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
			generateDesc(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
			generateDesc(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
		};
	}

	struct PushConstBlock {
		glm::vec2 scale;
		glm::vec2 translate;
	};

	void ImGuiMaterial::getPushConstantRange(std::vector<VkPushConstantRange>& pushConstantRanges)
	{
		pushConstantRanges.resize(1);
		VkPushConstantRange& pushConstantRange = pushConstantRanges[0];
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstBlock);
	}
}
