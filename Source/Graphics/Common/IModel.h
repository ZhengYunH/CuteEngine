#pragma once
#include <vector>
#include "Common/Config.h"
#include "IMesh.h"


namespace zyh
{
	class IModel
	{
	public:
		IModel(EPrimitiveType primType, const std::string& InModelFileName)
		{
			mMesh_ = new IMesh(primType, InModelFileName);
		}

		virtual ~IModel() 
		{ 
			SafeDestroy(mMesh_); 
		}
		
	public:
		void UpdateTransform(Matrix4x3& mat) { mMesh_->UpdateEntityTransform(mat); }

	public:
		IMesh* mMesh_;
	};

}