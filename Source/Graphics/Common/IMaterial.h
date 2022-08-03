#pragma once
#include "Common/Config.h"

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
			mShaderMap_[EShaderType::VS] = vertShaderFile;
			mShaderMap_[EShaderType::PS] = fragShaderFile;
		}
		virtual ~IMaterial() {}
		virtual bool IsValid() const { return true; }
		virtual const TRenderSets& GetSupportRenderSet() const { return mRenderSets_; }
		
		std::string GetShaderFilePath(EShaderType _Type) noexcept
		{
			HYBRID_CHECK(mShaderMap_.find(_Type) != mShaderMap_.end());
			return mShaderMap_[_Type];
		}

	public:
		TRenderSets mRenderSets_{ SCENE };

	protected:
		std::map<EShaderType, std::string> mShaderMap_;
	};
}