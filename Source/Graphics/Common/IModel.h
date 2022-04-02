#pragma once
#include <vector>
#include "Common/Config.h"
#include "IMesh.h"
#include "Math/Matrix4x3.h"


namespace zyh
{
	class IModel
	{
	public:
		IModel() 
		{
			
		}
		virtual ~IModel() 
		{ 
			SafeDestroy(mMesh_); 
		}

	public:
		IMesh* mMesh_;

	public:
		virtual IPrimitive* GeneratePrimitive()
		{
			return new IPrimitive();
		}

		void AddPrimitive(IPrimitive* prim, Matrix4x3* localTransform =nullptr)
		{ 
			mMesh_->AddPrimitive(prim, localTransform);
		}
	};

}