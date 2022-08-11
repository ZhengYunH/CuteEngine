#pragma once
#include <string>
#include <vector>
#include <fbxsdk.h>

#include "Common/Config.h"
#include "Geometry.h"


namespace zyh
{
	namespace FbxResourceLoader
	{
		void loadModel(const std::string& modelPath, std::vector<Vertex>& outVertexs, std::vector<uint32_t>& outIndices);
	};
}