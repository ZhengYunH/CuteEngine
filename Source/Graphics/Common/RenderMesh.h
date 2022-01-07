#pragma once
#include "Common/Config.h"
#include "Graphics/Common/Geometry.h"
#include "Graphics/Common/ResourceLoader.h"


namespace zyh
{
	class RenderMesh
	{
		RenderMesh(std::string meshFileName)
		{
			ResourceLoader::loadModel(meshFileName, vertices, indices);
		}

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};
}
