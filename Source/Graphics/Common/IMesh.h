#pragma once
#include <vector>
#include <string>

#include "Common/Config.h"
#include "IPrimitive.h"
#include "Math/Matrix4x4.h"

namespace zyh
{
	class IMesh
	{
	public:
		IMesh(const std::string& InMeshFileName) 
		{
			std::string primFileName = InMeshFileName;
			AddPrimitive(new IPrimitive(primFileName));
		}
		virtual ~IMesh() 
		{
			for (auto prim : mPrimitives_)
				SafeDestroy(prim);
			mPrimitives_.clear();
		}

	public:
		const std::string& GetName() const
		{ 
			return mName_; 
		}

		virtual size_t AddPrimitive(IPrimitive* prim, Matrix4x3* localTransform=nullptr) 
		{ 
			mPrimitives_.push_back(prim); 
			if (localTransform)
				mPrimitivesTransform_.push_back(*localTransform);
			else
				mPrimitivesTransform_.push_back(Matrix4x3());
			return mPrimitives_.size() - 1;
		}

		void GetPrimLocalTransform(size_t index, Matrix4x3& outMatrix)
		{
			HYBRID_CHECK(index < mPrimitivesTransform_.size());
			outMatrix = mPrimitivesTransform_[index];
		}

	public:
		std::vector<IPrimitive*> mPrimitives_;
		std::vector<Matrix4x3> mPrimitivesTransform_;
		std::string mName_;
	};
}