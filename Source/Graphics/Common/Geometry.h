#pragma once
#include "Graphics/Vulkan/VulkanHeader.h"

/// <summary>
/// Define Basic Geometry Data
/// </summary>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 texCoord;

	Vertex() {}
	Vertex(glm::vec3 inPos) : pos(inPos) {}
	Vertex(glm::vec3 inPos, glm::vec3 inColor, glm::vec2 inTexCoord) 
		: pos(inPos),  color(inColor), texCoord(inTexCoord)
	{}


	static void GetBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) {
		descriptions = {
			initInputBindingDesc(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
		};
	}

	static void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
	{
		descriptions = {
			initInputAttrDesc(0, 0, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(Vertex, pos)), // pos
			initInputAttrDesc(0, 1, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(Vertex, color)), // color
			initInputAttrDesc(0, 2, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(Vertex, normal)), // normal
			initInputAttrDesc(0, 3, VK_FORMAT_R32G32_SFLOAT,  offsetof(Vertex, texCoord)), // texCoord
		};
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

// Hash trait For Vertex
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}
