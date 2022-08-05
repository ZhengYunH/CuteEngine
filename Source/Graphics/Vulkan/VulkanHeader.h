#pragma once
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <array>
#include <map>
#include <optional>
#include <set>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <fstream>
#include <chrono>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

VkVertexInputBindingDescription initInputBindingDesc(uint32_t binding, uint32_t stride, VkVertexInputRate rate);
VkVertexInputAttributeDescription initInputAttrDesc(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);