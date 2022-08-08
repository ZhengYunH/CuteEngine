#include "VulkanShader.h"

namespace zyh
{
	ShaderFactory* GShaderCreator = new VulkanShaderFactory();

	void VulkanShaderParser::_GenerateInputVariable()
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
			if (inputVar->semantic)
				mInputVariable_[i].Semantic = inputVar->semantic;

			// build numeric
			{
				mInputVariable_[i].Numeric.Scalar.width = inputVar->numeric.scalar.width;
				mInputVariable_[i].Numeric.Scalar.signedness = inputVar->numeric.scalar.signedness;

				switch (inputVar->type_description->op)
				{
				case SpvOpTypeBool:
				case SpvOpTypeInt:
				case SpvOpTypeFloat:
					mInputVariable_[i].Numeric.Type = SShaderInputNumericType::SCALER;
					break;
				case SpvOpTypeVector:
					mInputVariable_[i].Numeric.Type = SShaderInputNumericType::VECTOR;
					mInputVariable_[i].Numeric.Block.Vector.component_count = inputVar->numeric.vector.component_count;
					break;
				case SpvOpTypeMatrix:
					mInputVariable_[i].Numeric.Block.Matrix.row_count = inputVar->numeric.matrix.row_count;
					mInputVariable_[i].Numeric.Block.Matrix.column_count = inputVar->numeric.matrix.column_count;
					mInputVariable_[i].Numeric.Block.Matrix.stride = inputVar->numeric.matrix.stride;
					break;
				default:
					Unimplement(0);
					break;
				}
			}
		}
	}

	void VulkanShaderParser::_GenerateDescriptor()
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

}