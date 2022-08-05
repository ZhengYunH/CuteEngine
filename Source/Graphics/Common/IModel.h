#pragma once
#include <vector>
#include "Common/Config.h"
#include "IMesh.h"


namespace zyh
{
	class IModel
	{
	public:
		IModel()
		{
			mMesh_ = new IMesh();
		}

		virtual ~IModel() 
		{ 
			SafeDestroy(mMesh_); 
		}
		
	public:
		void UpdateTransform(Matrix4x3& mat) { mMesh_->UpdateEntityTransform(mat); }
		void LoadResourceFile(const std::string& InFileName)
		{
			IPrimitive* prim = mMesh_->LoadResourceFile(InFileName);
			AddPrimitive(prim);
		}
		
		virtual size_t AddPrimitive(IPrimitive* prim, Matrix4x3* localTransform = nullptr)
		{
			return mMesh_->AddPrimitive(prim, localTransform);
		}

		virtual size_t AddPrimitive(EPrimitiveType primType, Matrix4x3* localTransform = nullptr)
		{
			return mMesh_->AddPrimitive(primType, localTransform);
		}

	public:
		IMesh* mMesh_;
	};

}