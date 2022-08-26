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
#include "VulkanShader.h"
#include "VulkanDescriptor.h"
#include "Math/Matrix4x4.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapchain.h"

#include "Graphics/Imgui/imgui.h"
#include "Graphics/Imgui/imgui_impl_vulkan.h"
#include "Graphics/Imgui/imgui_impl_win32.h"


namespace zyh
{
	VulkanMaterial::VulkanMaterial(IMaterial* material, RenderSet renderSet)
	{
		mMaterial_ = material;
		mRenderSet_ = renderSet;
		mGraphicsPipeline_ = new VulkanGraphicsPipeline(this);
		mDescriptorLayout_ = new VulkanDescriptorLayout(this);
	}

	VulkanMaterial::VulkanMaterial(IPrimitive* prim, RenderSet renderSet)
		: VulkanMaterial(prim->GetMaterial(), renderSet)
	{
		mPrim_ = prim;
	}

	void VulkanMaterial::connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, uint32_t layoutCount)
	{
		mPhysicalDevice_ = physicalDevice;
		mLogicalDevice_ = logicalDevice;
		mLayoutCount_ = layoutCount;
		mGraphicsPipeline_->connect(mLogicalDevice_);
		mDescriptorLayout_->connect(mLogicalDevice_);
	}

	void VulkanMaterial::setup()
	{
		createDescriptorSetData();
		createDescriptorLayout();
		createGraphicsPipeline();
		if (mUniformBuffers_.size() + mTextureImages_.size() > 0)
		{
			createDesciptorPool();
			createDescriptorSets();
		}
	}

	void VulkanMaterial::createGraphicsPipeline()
	{
		mGraphicsPipeline_->setup();
	}

	void VulkanMaterial::createDescriptorLayout()
	{
		mDescriptorLayout_->setup();
	}

	void VulkanMaterial::createDescriptorSetData()
	{
		IShaderParser* vsParser =  mMaterial_->GetShader(EShaderType::VS, mRenderSet_)->GetParser();
		IShaderParser* psParser = mMaterial_->GetShader(EShaderType::PS, mRenderSet_)->GetParser();

		_createDescriptorSetData(EShaderType::VS, vsParser);
		_createDescriptorSetData(EShaderType::PS, psParser);
	}

	void VulkanMaterial::_createDescriptorSetData(EShaderType shaderType, IShaderParser* parser)
	{
		VkShaderStageFlagBits stateBit;
		switch (shaderType)
		{
		case EShaderType::VS:
			stateBit = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case EShaderType::PS:
			stateBit = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		default:
			Unimplement(0);
			break;
		}

		auto& descriptors = parser->GetDescriptor();
		// TODO: support multi-set, now only one
		for (auto& setPair : descriptors)
		{
			uint32_t setIdx = setPair.first;
			for (auto& descPair : setPair.second)
			{
				uint32_t binding = descPair.first;
				const SShaderDescriptorData& desc = descPair.second;

				if (desc.Type == EDescriptorType::UNIFORM_BUFFER)
				{
					EUniformType type{ EUniformType::NONE };
					if (desc.Name == "Batch")
						type = EUniformType::BATCH;
					if (desc.Name == "Light")
						type = EUniformType::LIGHT;

					if (type == EUniformType::NONE)
						continue;

					if (mUniformBuffers_.find(binding) != mUniformBuffers_.end())
					{
						HYBRID_CHECK(mUniformBuffers_.at(binding)[0]->GetBufferSize() == desc.Block.Uniform.Size);
						for (size_t i = 0; i < mLayoutCount_; ++i)
						{
							mUniformBuffers_.at(binding)[i]->AddState(stateBit);
						}
					}
					std::vector<VulkanUniformBuffer*> uniformBuffers;
					uniformBuffers.resize(mLayoutCount_);
					for (auto& uniformBuffer : uniformBuffers)
					{
						uniformBuffer = new VulkanUniformBuffer(type, stateBit);
						uniformBuffer->connect(mPhysicalDevice_, mLogicalDevice_);
						uniformBuffer->setup(desc.Block.Uniform.Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
					}
					mUniformBuffers_[binding] = std::move(uniformBuffers);
				}

				if (desc.Type == EDescriptorType::SAMPLER)
				{
					VulkanTextureImage* baseTextureImage_ = new VulkanTextureImage("Resource/textures/viking_room.png");
					baseTextureImage_->connect(mPhysicalDevice_, mLogicalDevice_, GVulkanInstance->mGraphicsCommandPool_);
					baseTextureImage_->setup(VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
						VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
					);

					mTextureImages_[binding] = baseTextureImage_;
				}
			}
		}
	}

	void VulkanMaterial::createDesciptorPool()
	{
		std::vector<VkDescriptorPoolSize> poolSizes;
		poolSizes.resize(mUniformBuffers_.size() + mTextureImages_.size());

		for (auto& uniformPair : mUniformBuffers_)
		{
			VkDescriptorPoolSize& poolSize = poolSizes[uniformPair.first];
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = static_cast<uint32_t>(mLayoutCount_);
		}

		for (auto& texturePair : mTextureImages_)
		{
			VkDescriptorPoolSize& poolSize = poolSizes[texturePair.first];
			poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSize.descriptorCount = static_cast<uint32_t>(mLayoutCount_);
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(mLayoutCount_);

		VK_CHECK_RESULT(vkCreateDescriptorPool(mLogicalDevice_->Get(), &poolInfo, nullptr, &mDescriptorPool_), "failed to create descriptor pool!");
	}

	void VulkanMaterial::createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(mLayoutCount_, mDescriptorLayout_->Get());

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
		}
	}

	void VulkanMaterial::cleanup()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	void VulkanMaterial::updateUniformBuffer(UniformBufferObject& ubo, UniformLightingBufferObject& ulbo)
	{
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

	void ImGuiMaterial::createDescriptorSetData()
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
		descriptions =
		{
			std::move(initInputBindingDesc(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX))
		};
	}

	void ImGuiMaterial::getAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
	{
		descriptions = 
		{
			initInputAttrDesc(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
			initInputAttrDesc(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
			initInputAttrDesc(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
		};
	}

	void ImGuiMaterial::getPushConstantRange(std::vector<VkPushConstantRange>& pushConstantRanges)
	{
		pushConstantRanges.resize(1);
		VkPushConstantRange& pushConstantRange = pushConstantRanges[0];
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstBlock);
	}

	void ImGuiMaterial::PushConstant(std::string semantic, void* data)
	{
		if (semantic == "scale")
		{
			float* fdata = (float*)data;
			pushConstBlock.scale = glm::vec2(fdata[0], fdata[1]);
		}

		else if (semantic == "translate")
		{
			float* fdata = (float*)data;
			pushConstBlock.translate = glm::vec2(fdata[0], fdata[1]);
		}
	}

	void ImGuiMaterial::BindPushConstant(VkCommandBuffer vkCommandBuffer)
	{
		vkCmdPushConstants(vkCommandBuffer, getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);
	}
}
