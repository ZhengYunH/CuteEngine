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
		IMesh(EPrimitiveType primType, const std::string& InMeshFileName)
		{
			switch (primType)
			{
			case zyh::MESH:
				HYBRID_CHECK(!InMeshFileName.empty());
				AddPrimitive(new IPrimitive(InMeshFileName));
				break;
			case zyh::BOX:
				Unimplement(0);
				break;
			case zyh::SPHERE:
				AddPrimitive(new SpherePrimitive());
				break;
			default:
				Unimplement(0);
				break;
			}
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

		void UpdateEntityTransform(Matrix4x3& mat) 
		{ 
			EntityTransform_ = mat;
			for (size_t index =0; index < mPrimitives_.size(); ++index)
			{
				IPrimitive* prim = mPrimitives_[index];
				prim->SetTransform(EntityTransform_); // *PrimitivesTransform_[index]);
			}
		}

	public:
		std::vector<IPrimitive*> mPrimitives_;
		std::vector<Matrix4x3> mPrimitivesTransform_;
		Matrix4x3 EntityTransform_;
		std::string mName_;
	};
}