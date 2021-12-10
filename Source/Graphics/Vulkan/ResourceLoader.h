#pragma once
#include "VulkanHeader.h"
#include "Geometry.h"


namespace ResourceLoader {
	std::vector<char> readFile(const std::string& filename);
	void loadModel(const std::string& modelPath, std::vector<Vertex>& outVertexs, std::vector<uint32_t>& outIndices);
}