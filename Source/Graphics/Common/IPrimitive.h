#pragma once
#include "Graphics/Common/Geometry.h"
#include "Graphics/Common/ResourceLoader.h"
#include "IMaterial.h"

namespace zyh
{
	class IPrimitive
	{
	public:
		IPrimitive(const std::string& InPrimFileName)
		{
			mMaterial_ = new IMaterial();
			ResourceLoader::loadModel(InPrimFileName, mVertices_, mIndices_);
		}

		virtual ~IPrimitive() 
		{ 
			SafeDestroy(mMaterial_); 
		}

	public:
		const IMaterial& GetMaterial() 
		{ 
			return *mMaterial_; 
		}

	protected:
		IMaterial* mMaterial_;

	public:
		std::vector<Vertex> mVertices_;
		std::vector<uint32_t> mIndices_;
	};
}