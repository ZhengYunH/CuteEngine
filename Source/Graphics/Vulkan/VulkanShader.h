#pragma once
#include "Common/Config.h"
#include "Graphics/Common/Shader.h"
#include "Graphics/Common/ResourceLoader.h"

#include "VulkanBase.h"
#include "VulkanLogicalDevice.h"
#include "spirv_reflect.h"


namespace zyh
{
	class VulkanShaderParser : public IShaderParser
	{
	public:
		VulkanShaderParser(const void* spirv_code, size_t spirv_nbytes) : IShaderParser()
		{
			SpvReflectResult result;
			
			result = spvReflectCreateShaderModule(spirv_nbytes, spirv_code, &mModule_);
			HYBRID_CHECK(result == SPV_REFLECT_RESULT_SUCCESS);

			_GenerateInputVariable();
			_GenerateDescriptor();
		}

		~VulkanShaderParser()
		{
			spvReflectDestroyShaderModule(&mModule_);
		}

		virtual void _GenerateInputVariable() override
		{
			HYBRID_CHECK(mInputVariable_.empty());

			SpvReflectResult result;

			uint32_t varCount = 0;
			result = spvReflectEnumerateInputVariables(&mModule_, &varCount, NULL);
			HYBRID_CHECK(result == SPV_REFLECT_RESULT_SUCCESS);

			SpvReflectInterfaceVariable** inputVars = (SpvReflectInterfaceVariable**)malloc(varCount * sizeof(SpvReflectInterfaceVariable*));
			result = spvReflectEnumerateInputVariables(&mModule_, &varCount, inputVars);
			HYBRID_CHECK(result == SPV_REFLECT_RESULT_SUCCESS);

			mInputVariable_.resize(varCount);
			for (size_t i = 0; i < varCount; ++i)
			{
				SpvReflectInterfaceVariable* inputVar = inputVars[i];
				mInputVariable_[i].Name = inputVar->name;
				mInputVariable_[i].Location = inputVar->location;
				if(inputVar->semantic)
					mInputVariable_[i].Semantic = inputVar->semantic;

				// build numeric
				{
					mInputVariable_[i].Numeric.scalar.width = inputVar->numeric.scalar.width;
					mInputVariable_[i].Numeric.scalar.signedness = inputVar->numeric.scalar.signedness;

					mInputVariable_[i].Numeric.vector.component_count = inputVar->numeric.vector.component_count;

					mInputVariable_[i].Numeric.matrix.column_count = inputVar->numeric.matrix.column_count;
					mInputVariable_[i].Numeric.matrix.row_count = inputVar->numeric.matrix.row_count;
					mInputVariable_[i].Numeric.matrix.stride = inputVar->numeric.matrix.stride;
				}
			}
		}

		virtual void _GenerateDescriptor() override
		{
			SpvReflectResult result;

			uint32_t descSetCount = 0;
			result = spvReflectEnumerateDescriptorSets(&mModule_, &descSetCount, NULL);
			HYBRID_CHECK(result == SPV_REFLECT_RESULT_SUCCESS);
			SpvReflectDescriptorSet** inputSets = (SpvReflectDescriptorSet**)malloc(descSetCount * sizeof(SpvReflectDescriptorSet*));
			result = spvReflectEnumerateDescriptorSets(&mModule_, &descSetCount, inputSets);
			HYBRID_CHECK(result == SPV_REFLECT_RESULT_SUCCESS);

			for (size_t i = 0; i < descSetCount; ++i)
			{
				SpvReflectDescriptorSet* inputSet = inputSets[i];
				mDescriptor_[inputSet->set] = std::unordered_map<uint32_t, SShaderDescriptorData>();
				auto& descs = mDescriptor_[inputSet->set];
				for (size_t j = 0; j < inputSet->binding_count; ++j)
				{
					SpvReflectDescriptorBinding* inDescBinding = inputSet->bindings[j];
					SShaderDescriptorData& desc = descs[inDescBinding->binding] = SShaderDescriptorData();
					desc.Name = inDescBinding->name;

					switch (inDescBinding->descriptor_type)
					{
					case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
					case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
						desc.Type = EDescriptorType::SAMPLER;
						break;
					case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
						desc.Type = EDescriptorType::UNIFORM_BUFFER;
						desc.Block.Uniform.Size = inDescBinding->block.size;
						break;
					default:
						Unimplement(0);
						break;
					}
				}
			}
		}

	protected:
		SpvReflectShaderModule mModule_;
	};


	class VulkanShader : public IShader
	{
	public: 
		VulkanShader(const std::string& shaderFilePath) : IShader(shaderFilePath)
		{
			CreateParser();
		}

		virtual ~VulkanShader()
		{
			vkDestroyShaderModule(GVulkanInstance->mLogicalDevice_->Get(), mShaderModule_, nullptr);
		}

	public:
		virtual void CreateParser()
		{
			auto code = ResourceLoader::readFile(mFilePath_);
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			VK_CHECK_RESULT(
				vkCreateShaderModule(GVulkanInstance->mLogicalDevice_->Get(), &createInfo, nullptr, &mShaderModule_),
				"failed to create shader module!"
			);

			mParser_ = new VulkanShaderParser(code.data(), code.size());
		}

		VkShaderModule& GetShaderModule() { return mShaderModule_; }

	protected:
		VkShaderModule mShaderModule_;

	};


	class VulkanShaderFactory : public ShaderFactory
	{
	public:
		virtual VulkanShader* _CreateShader(const std::string& shaderFilePath) override
		{
			return new VulkanShader(shaderFilePath);
		}
	};
}