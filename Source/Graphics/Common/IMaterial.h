#pragma once
#include "Common/Config.h"
#include "Graphics/Common/RenderStage.h"
#include "Graphics/Common/Shader.h"


namespace zyh
{
	namespace ShaderType
	{
		enum EShaderType
		{
			NONE = 0,
			VS = 1,
			PS = 2,
			COMPUTE = 3,
		};
	}
	using EShaderType = ShaderType::EShaderType;

	class IMaterial
	{
	public:
		IMaterial(const std::string& vertShaderFile, const std::string& fragShaderFile)
		{
			mShaderPathMap_[EShaderType::VS] = vertShaderFile;
			mShaderPathMap_[EShaderType::PS] = fragShaderFile;
		}
		virtual ~IMaterial() {}
		virtual bool IsValid() const { return true; }
		virtual const TRenderSets& GetSupportRenderSet() const { return mRenderSets_; }
		IShader* GetShader(EShaderType _Type) noexcept
		{
			return GShaderCreator->GetShader(mShaderPathMap_[_Type]);
		}

	public:
		IPipelineState& GetPipelineState() { return mPipelineState_; }

	protected:
		TRenderSets mRenderSets_{ SCENE };
		IPipelineState mPipelineState_{};
		DepthStencilState DepthStencil{};
		RasterizationState Rasterization{};

	protected:
		std::map<EShaderType, std::string> mShaderPathMap_;
		std::map<EShaderType, IShader*> mShaderMap_;
	};
}