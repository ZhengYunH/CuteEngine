#pragma once
#include <vector>
#include "Common/Config.h"
#include "IMesh.h"


namespace zyh
{
	class IModel
	{
	public:
		IModel(const std::string& InModelFileName)
		{
			std::string meshFileName = InModelFileName;
			mMesh_ = new IMesh(meshFileName);
		}
		virtual ~IModel() 
		{ 
			SafeDestroy(mMesh_); 
		}
		
	public:
		IMesh* mMesh_;
	};

}