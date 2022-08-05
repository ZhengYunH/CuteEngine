#include "ResourceLoader.h"
#include "ObjResourceLoader.h"
#include "FbxResourceLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace zyh
{
	namespace ResourceLoader {
		std::vector<char> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				HYBRID_CHECK(0);
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();

			return buffer;
		}

		void loadModel(const std::string& modelPath, std::vector<Vertex>& outVertexs, std::vector<uint32_t>& outIndices)
		{
			size_t index = modelPath.rfind('.');
			if (index == std::string::npos)
			{
				HYBRID_CHECK(0, "Load Model Must Include PostFix");
			}
			std::string postFix = modelPath.substr(index, std::string::npos);
			if (postFix == ".obj")
			{
				ObjResourceLoader::loadModel(modelPath, outVertexs, outIndices);
			}
			else if (postFix == ".fbx")
			{
				FbxResourceLoader::loadModel(modelPath, outVertexs, outIndices);
			}
			else
			{
				assert(0);
			}
		}
	}
}