#pragma once
#include "Common/Config.h"
#include "Geometry.h"
#include "IModel.h"

namespace zyh
{
	namespace ResourceLoader {
		std::vector<char> readFile(const std::string& filename);
		void loadModel(const std::string& modelPath, IModel& model);
		void loadModel(const std::string& modelPath, std::vector<Vertex>& outVertexs, std::vector<uint32_t>& outIndices);
	}
}
