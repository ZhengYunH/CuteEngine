#pragma once
#include "Common/Config.h"
#include "Graphics/Common/Geometry.h"
#include "Graphics/Common/ResourceLoader.h"
#include "Graphics/Common/IModel.h"


namespace zyh
{
	class IRenderElement
	{
	public:
		IRenderElement()
		{
		}

		IRenderElement(const std::string& meshFileName)
		{
			mModel_ = new IModel();
			LoadModel(meshFileName);
		}

		void LoadModel(const std::string& meshFileName)
		{
			ResourceLoader::loadModel(meshFileName, *mModel_);
		}

	protected:
		IModel* mModel_;
		std::vector<Vertex> mVertices_;
		std::vector<uint32_t> mIndices_;
	};
}
