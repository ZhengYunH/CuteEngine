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
		IMaterial(const std::string& vertShaderFile, const std::string& fragShaderFile, RenderSet set = RenderSet::SCENE)
		{
			mShaderPathMap_[set] = std::map<EShaderType, std::string>();
			mShaderPathMap_[set][EShaderType::VS] = vertShaderFile;
			mShaderPathMap_[set][EShaderType::PS] = fragShaderFile;
		}
		virtual ~IMaterial() {}
		virtual bool IsValid() const { return true; }
		virtual const TRenderSets& GetSupportRenderSet() const { return mRenderSets_; }
		void AddRenderSet(RenderSet set, const std::string& vertShaderFile, const std::string& fragShaderFile)
		{ 
			if (std::find(mRenderSets_.begin(), mRenderSets_.end(), set) == mRenderSets_.end())
			{
				mRenderSets_.push_back(set);
				mShaderPathMap_[set] = std::map<EShaderType, std::string>();
				mShaderPathMap_[set][EShaderType::VS] = vertShaderFile;
				mShaderPathMap_[set][EShaderType::PS] = fragShaderFile;
			}
		}

		IShader* GetShader(EShaderType _Type, RenderSet renderSet = RenderSet::SCENE) noexcept
		{
			HYBRID_CHECK(mShaderPathMap_.find(renderSet) != mShaderPathMap_.end());
			return GShaderCreator->GetShader(mShaderPathMap_[renderSet][_Type]);
		}

	public:
		IPipelineState& GetPipelineState() { return mPipelineState_; }

	protected:
		TRenderSets mRenderSets_{ SCENE };
		IPipelineState mPipelineState_{};

	protected:
		std::unordered_map<RenderSet, std::map<EShaderType, std::string>> mShaderPathMap_{};
	};
}