#include "VulkanInstance.h"

#include "VulkanGraphicsPipeline.h"
#include "VulkanLogicalDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanMaterial.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"


namespace zyh
{
	void VulkanGraphicsPipelineBase::connect(VulkanLogicalDevice* logicalDevice)
	{
		mVulkanLogicalDevice = logicalDevice;
	}

	void VulkanGraphicsPipelineBase::setup()
	{
		// only create pipeline cache, concrete setup will be done at child object
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreatePipelineCache(mVulkanLogicalDevice->Get(), &pipelineCacheCreateInfo, nullptr, &mVkPipelineCache_));
	}

	void VulkanGraphicsPipeline::connect(VulkanLogicalDevice* logicalDevice, VulkanRenderPassBase* renderPass)
	{
		VulkanGraphicsPipelineBase::connect(logicalDevice);
		mVulkanRenderPass_ = renderPass;
	}

	void VulkanGraphicsPipeline::setup()
	{
		VulkanGraphicsPipelineBase::setup();
		_setupDescriptorSetLayout();
		_setupGraphicsPipeline();
	}

	void VulkanGraphicsPipeline::cleanup()
	{
		vkDestroyPipeline(mVulkanLogicalDevice->Get(), mVkImpl_, nullptr);
		vkDestroyPipelineLayout(mVulkanLogicalDevice->Get(), mVkPipelineLayout_, nullptr);
		vkDestroyDescriptorSetLayout(mVulkanLogicalDevice->Get(), mVkDescriptorSetLayout_, nullptr);
	}

	void VulkanGraphicsPipeline::_setupDescriptorSetLayout()
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
			vkCreateDescriptorSetLayout(mVulkanLogicalDevice->Get(), &layoutInfo, nullptr, &mVkDescriptorSetLayout_), 
			"failed to create descriptor set layout!"
		);
	}

	void VulkanGraphicsPipeline::_setupGraphicsPipeline()
	{
		VkShaderModule vertShaderModule = static_cast<VulkanShader*>(mOwner_->mMaterial_->GetShader(EShaderType::VS, mOwner_->mRenderSet_))->GetShaderModule();
		VkShaderModule fragShaderModule = static_cast<VulkanShader*>(mOwner_->mMaterial_->GetShader(EShaderType::PS, mOwner_->mRenderSet_))->GetShaderModule();

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


		// Vertex Input
		std::vector<VkVertexInputBindingDescription> bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		mOwner_->getBindingDescriptions(bindingDescription);
		mOwner_->getAttributeDescriptions(attributeDescriptions);

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;


		// View-ports and Scissors
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)GInstance->mExtend_->width;
		viewport.height = (float)GInstance->mExtend_->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = *(GInstance->mExtend_);

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// rasterizer
		RasterizationState rasterizationState = mOwner_->GetRasterizationState();
		VkCullModeFlags cullmode = VK_CULL_MODE_NONE;
		switch (rasterizationState.CullMode)
		{
		case ERasterizationCullMode::NONE:
			cullmode = VK_CULL_MODE_NONE;
			break;
		case ERasterizationCullMode::FRONT:
			cullmode = VK_CULL_MODE_FRONT_BIT;
			break;
		case ERasterizationCullMode::BACK:
			cullmode = VK_CULL_MODE_BACK_BIT;
			break;
		case ERasterizationCullMode::FRONT_AND_BACK:
			cullmode = VK_CULL_MODE_FRONT_AND_BACK;
			break;
		default:
			Unimplement(0);
			break;
		}

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = cullmode;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


		// Multi-sampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
		multisampling.rasterizationSamples = *(GInstance->mMsaaSamples_);
		multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smooth
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional


		// Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional


		// Pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &mVkDescriptorSetLayout_;

		std::vector<VkPushConstantRange> pushConstantRanges;
		mOwner_->getPushConstantRange(pushConstantRanges);
		if (pushConstantRanges.empty())
		{
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;
		}
		else
		{
			pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
			pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
		}
		

		if (vkCreatePipelineLayout(mVulkanLogicalDevice->Get(), &pipelineLayoutInfo, nullptr, &mVkPipelineLayout_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		DepthStencilState depthStencilState = mOwner_->GetDepthStencilState();
		
		// Depth and stencil state
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = depthStencilState.DepthTestEnable;
		depthStencil.depthWriteEnable = depthStencilState.DepthWriteEnable;
		depthStencil.depthCompareOp = _convertCompareOp(depthStencilState.DepthCompareOp);
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = depthStencilState.StencilTestEnable;

		depthStencil.front.failOp = _convertStencilOp(depthStencilState.StencilState.FailOp);
		depthStencil.front.passOp = _convertStencilOp(depthStencilState.StencilState.PassOp);
		depthStencil.front.depthFailOp = _convertStencilOp(depthStencilState.StencilState.DepthFailOp);
		depthStencil.front.compareOp = _convertCompareOp(depthStencilState.StencilState.CompareOp);
		depthStencil.front.compareMask = 0xff;
		depthStencil.front.writeMask = 0xff;
		depthStencil.front.reference = depthStencilState.StencilState.Reference;
		depthStencil.back = depthStencil.front;

		// Dynamic State
		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
		pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
		pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		pipelineDynamicStateCreateInfo.flags = 0;

		// Create real pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = mVkPipelineLayout_;
		pipelineInfo.renderPass = mVulkanRenderPass_->Get();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pDynamicState = &pipelineDynamicStateCreateInfo;

		if (vkCreateGraphicsPipelines(mVulkanLogicalDevice->Get(), mVkPipelineCache_, 1, &pipelineInfo, nullptr, &mVkImpl_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}

	VkCompareOp VulkanGraphicsPipeline::_convertCompareOp(ECompareOP op)
	{
		VkCompareOp compareOp = VK_COMPARE_OP_NEVER;
		switch (op)
		{
		case ECompareOP::NEVER:
			compareOp = VK_COMPARE_OP_NEVER;
			break;
		case ECompareOP::EQUAL:
			compareOp = VK_COMPARE_OP_EQUAL;
			break;
		case ECompareOP::GREATER:
			compareOp = VK_COMPARE_OP_GREATER;
			break;
		case ECompareOP::LESS_OR_EQUAL:
			compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			break;
		case ECompareOP::NOT_EQUAL:
			compareOp = VK_COMPARE_OP_NOT_EQUAL;
			break;
		case ECompareOP::GREATER_OR_EQUAL:
			compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
			break;
		case ECompareOP::ALWAYS:
			compareOp = VK_COMPARE_OP_ALWAYS;
			break;
		default:
			Unimplement();
			break;
		}
		return compareOp;
	}

	VkStencilOp VulkanGraphicsPipeline::_convertStencilOp(DepthStencilState::EStencilOp op)
	{
		VkStencilOp stencilOp = VK_STENCIL_OP_KEEP;
		switch (op)
		{
		case DepthStencilState::EStencilOp::KEEP:
			stencilOp = VK_STENCIL_OP_KEEP;
			break;
		case DepthStencilState::EStencilOp::ZERO:
			stencilOp = VK_STENCIL_OP_ZERO;
			break;
		case DepthStencilState::EStencilOp::REPLACE:
			stencilOp = VK_STENCIL_OP_REPLACE;
			break;
		case DepthStencilState::EStencilOp::INCREMENT_AND_CLAMP:
			stencilOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
			break;
		case DepthStencilState::EStencilOp::DECREMENT_AND_CLAMP:
			stencilOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
			break;
		case DepthStencilState::EStencilOp::INVERT:
			stencilOp = VK_STENCIL_OP_INVERT;
			break;
		case DepthStencilState::EStencilOp::INCREMENT_AND_WRAP:
			stencilOp = VK_STENCIL_OP_INCREMENT_AND_WRAP;
			break;
		case DepthStencilState::EStencilOp::DECREMENT_AND_WRAP:
			stencilOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;
			break;
		default:
			Unimplement();
			break;
		}
		return stencilOp;
	}

}