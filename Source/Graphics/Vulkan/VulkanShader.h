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

		virtual void _GenerateInputVariable() override;

		virtual void _GenerateDescriptor() override;

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