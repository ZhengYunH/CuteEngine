#include "VulkanInstance.h"

#include "VulkanGraphicsPipeline.h"
#include "VulkanLogicalDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanMaterial.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "VulkanDescriptor.h"


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

	void VulkanGraphicsPipeline::connect(VulkanLogicalDevice* logicalDevice)
	{
		VulkanGraphicsPipelineBase::connect(logicalDevice);
	}

	void VulkanGraphicsPipeline::setup()
	{
		VulkanGraphicsPipelineBase::setup();
		_setupGraphicsPipeline();
	}

	void VulkanGraphicsPipeline::cleanup()
	{
		vkDestroyPipeline(mVulkanLogicalDevice->Get(), mVkImpl_, nullptr);
		vkDestroyPipelineLayout(mVulkanLogicalDevice->Get(), mVkPipelineLayout_, nullptr);
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
		const ColorBlendState& colorBlendState = mOwner_->GetColorBlendState();

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = colorBlendState.BlendEnable;
		colorBlendAttachment.srcColorBlendFactor = _convertBlendFactor(colorBlendState.SrcColorBlendFactor);
		colorBlendAttachment.dstColorBlendFactor = _convertBlendFactor(colorBlendState.DstColorBlendFactor);
		colorBlendAttachment.colorBlendOp = _convertBlendOp(colorBlendState.ColorBlendOp);
		colorBlendAttachment.srcAlphaBlendFactor = _convertBlendFactor(colorBlendState.SrcAlphaBlendFactor);;
		colorBlendAttachment.dstAlphaBlendFactor = _convertBlendFactor(colorBlendState.DstAlphaBlendFactor);;
		colorBlendAttachment.alphaBlendOp = _convertBlendOp(colorBlendState.AlphaBlendOp);

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
		pipelineLayoutInfo.pSetLayouts = &mOwner_->GetDescSetLayout()->Get();

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

		const DepthStencilState& depthStencilState = mOwner_->GetDepthStencilState();
		
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
		pipelineInfo.renderPass = mOwner_->GetRenderPass()->Get();
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
		switch (op)
		{
		case ECompareOP::NEVER:
			return VK_COMPARE_OP_NEVER;
		case ECompareOP::EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case ECompareOP::GREATER:
			return VK_COMPARE_OP_GREATER;
		case ECompareOP::LESS_OR_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case ECompareOP::NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case ECompareOP::GREATER_OR_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case ECompareOP::ALWAYS:
			return VK_COMPARE_OP_ALWAYS;
		default:
			Unimplement();
			break;
		}
		return VK_COMPARE_OP_MAX_ENUM;
	}

	VkStencilOp VulkanGraphicsPipeline::_convertStencilOp(DepthStencilState::EStencilOp op)
	{
		switch (op)
		{
		case DepthStencilState::EStencilOp::KEEP:
			return VK_STENCIL_OP_KEEP;
		case DepthStencilState::EStencilOp::ZERO:
			return VK_STENCIL_OP_ZERO;
		case DepthStencilState::EStencilOp::REPLACE:
			return VK_STENCIL_OP_REPLACE;
		case DepthStencilState::EStencilOp::INCREMENT_AND_CLAMP:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case DepthStencilState::EStencilOp::DECREMENT_AND_CLAMP:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
			break;
		case DepthStencilState::EStencilOp::INVERT:
			return VK_STENCIL_OP_INVERT;
		case DepthStencilState::EStencilOp::INCREMENT_AND_WRAP:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case DepthStencilState::EStencilOp::DECREMENT_AND_WRAP:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		default:
			Unimplement();
			break;
		}
		return VK_STENCIL_OP_MAX_ENUM;
	}

	VkBlendOp VulkanGraphicsPipeline::_convertBlendOp(ColorBlendState::EBlendOP op)
	{
		switch (op)
		{
		case ColorBlendState::EBlendOP::ADD:
			return VK_BLEND_OP_ADD;
		case ColorBlendState::EBlendOP::SUBSTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case ColorBlendState::EBlendOP::REVERSE_SUBSTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case ColorBlendState::EBlendOP::MIN:
			return VK_BLEND_OP_MIN;
		case ColorBlendState::EBlendOP::MAX:
			return VK_BLEND_OP_MAX;
		default:
			Unimplement(0);
			break;
		}
		return VK_BLEND_OP_MAX_ENUM;
	}

	VkBlendFactor VulkanGraphicsPipeline::_convertBlendFactor(ColorBlendState::EBlendFactor factor)
	{
		switch (factor)
		{
		case ColorBlendState::EBlendFactor::ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case ColorBlendState::EBlendFactor::ONE:
			return VK_BLEND_FACTOR_ONE;
		case ColorBlendState::EBlendFactor::SRC_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case ColorBlendState::EBlendFactor::ONE_MINUS_SRC_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case ColorBlendState::EBlendFactor::DST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case ColorBlendState::EBlendFactor::ONE_MINUS_DST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case ColorBlendState::EBlendFactor::SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case ColorBlendState::EBlendFactor::ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case ColorBlendState::EBlendFactor::DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case ColorBlendState::EBlendFactor::ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case ColorBlendState::EBlendFactor::CONSTANT_COLOR:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case ColorBlendState::EBlendFactor::ONE_MINUS_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case ColorBlendState::EBlendFactor::CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		case ColorBlendState::EBlendFactor::ONE_MINUS_CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		default:
			Unimplement(0);
			break;
		}
		return VK_BLEND_FACTOR_MAX_ENUM;
	}

}