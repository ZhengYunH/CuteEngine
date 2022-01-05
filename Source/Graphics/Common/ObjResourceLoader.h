#pragma once
#include <string>
#include <vector>

#include "Common/Config.h"
#include "Geometry.h"

namespace zyh
{
	namespace ObjResourceLoader
	{
		void loadModel(const std::string& modelPath, std::vector<Vertex>& outVertexs, std::vector<uint32_t>& outIndices);
	};
}