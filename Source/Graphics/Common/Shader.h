#pragma once
#include "Common/Config.h"

namespace zyh
{
	struct SShaderNumericTraits
	{
		struct Scalar {
			uint32_t                        width;
			uint32_t                        signedness;
		} scalar;

		struct Vector {
			uint32_t                        component_count;
		} vector;

		struct Matrix {
			uint32_t                        column_count;
			uint32_t                        row_count;
			uint32_t                        stride; // Measured in bytes
		} matrix;
	};

	struct SShaderInputVariableData
	{
		std::string Name;
		uint32_t Location;
		std::string Semantic;
		SShaderNumericTraits Numeric;
	};

	enum class EDescriptorType
	{
		SAMPLER = 0,
		UNIFORM_BUFFER = 1,
		INPUT_ATTACHMENT = 2
	};

	struct SShaderUniformTrait
	{
		uint32_t Size;
	};

	struct SShaderSamplerTrait
	{

	};

	struct SShaderDescriptorData
	{
		std::string Name;
		EDescriptorType Type;
		union
		{
			SShaderUniformTrait Uniform;
			SShaderSamplerTrait	Sampler;
		} Block;
	};

	class IShaderParser
	{
	public:
		IShaderParser() {}
		const std::vector<SShaderInputVariableData>& GetInputVariable()
		{
			if (mInputVariable_.empty())
			{
				_GenerateInputVariable();
			}
			return mInputVariable_;
		}

		const std::unordered_map<uint32_t, std::unordered_map<uint32_t, SShaderDescriptorData>>& GetDescriptor()
		{
			if (mDescriptor_.empty())
			{
				_GenerateDescriptor();
			}
			return mDescriptor_;
		}

	protected:
		virtual void _GenerateInputVariable() = 0;
		virtual void _GenerateDescriptor() = 0;

	protected:
		std::vector<SShaderInputVariableData> mInputVariable_;
		std::unordered_map<uint32_t/*set*/, std::unordered_map<uint32_t/* binding*/, SShaderDescriptorData>> mDescriptor_;
	};


	class IShader
	{
	public:
		IShader(const std::string& shaderFilePath) 
			: mFilePath_{ shaderFilePath }
		{
		}
		virtual ~IShader() { SafeDestroy(mParser_); }
	
	public:
		IShaderParser* GetParser()
		{ 
			HYBRID_CHECK(mParser_, "forget to create Parser?");
			return mParser_;
		}

	protected:
		std::string mFilePath_{};
		IShaderParser* mParser_{ nullptr };
	};

	class ShaderFactory
	{
	public:
		virtual IShader* GetShader(const std::string& shaderFilePath)
		{
			if (mShaderCache_.find(shaderFilePath) == mShaderCache_.end())
				mShaderCache_.insert({ shaderFilePath  ,_CreateShader(shaderFilePath) });
			return mShaderCache_[shaderFilePath];
		}
		virtual IShader* _CreateShader(const std::string& shaderFilePath) = 0;

	protected:
		std::unordered_map<std::string, IShader*> mShaderCache_;
	};

	extern ShaderFactory* GShaderCreator;
}