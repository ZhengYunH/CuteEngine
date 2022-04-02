#pragma once
#include "Graphics/Common/Geometry.h"
#include "IMaterial.h"


namespace zyh
{
	class IPrimitive
	{
	public:
		virtual ~IPrimitive() { SafeDestroy(mMaterial_); }
	public:
		IMaterial* mMaterial_;

		std::vector<Vertex> mVertices_;
		std::vector<uint32_t> mIndices_;
	};
}