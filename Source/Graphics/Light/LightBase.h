#pragma once
#include "Common/Config.h"
#include "Graphics/Vulkan/VulkanTools.h"
#include "Math/Matrix4x3.h"
#include "Graphics/Vulkan/VulkanHeader.h"

namespace zyh
{
	class DirectionLight
	{
	public:
		DirectionLight() {}

		DirectionLight(Vector3 Direction, Vector3 Ambient, Vector3 Diffuse, Vector3 Specular)
			: mDirection_(Direction.GetNormalized())
			, mAmbient_(Ambient)
			, mDiffuse_(Diffuse)
			, mSpecular_(Specular)
		{

		}

		alignas(16) Vector3 mDirection_;
		alignas(16) Vector3 mAmbient_;
		alignas(16) Vector3 mDiffuse_;
		alignas(16) Vector3 mSpecular_;

		static VkVertexInputBindingDescription GetBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) {
			descriptions = {
				initInputBindingDesc(0, sizeof(DirectionLight), VK_VERTEX_INPUT_RATE_VERTEX)
			};
		}

		static void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
		{
			descriptions = {
				initInputAttrDesc(0, 0, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(DirectionLight, mDirection_)), // mDirection_
				initInputAttrDesc(0, 1, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(DirectionLight, mAmbient_)), // mAmbient_
				initInputAttrDesc(0, 2, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(DirectionLight, mDiffuse_)), // mDiffuse_
				initInputAttrDesc(0, 3, VK_FORMAT_R32G32_SFLOAT,  offsetof(DirectionLight, mSpecular_)), // mSpecular_
			};
		}
	};

	class PointLight
	{
	public:
		PointLight() {}

		PointLight(Vector3 Position, Vector3 Ambient, Vector3 Diffuse, Vector3 Specular)
			: mPosition_(Position)
			, mAmbient_(Ambient)
			, mDiffuse_(Diffuse)
			, mSpecular_(Specular)
		{

		}

		alignas(16) Vector3 mPosition_;
		alignas(16) Vector3 mAmbient_;
		alignas(16) Vector3 mDiffuse_;
		alignas(16) Vector3 mSpecular_;

		alignas(4) float mConstant_{ 1.f };
		alignas(4) float mLinear_{ 0.7f };
		alignas(4) float mQuadratic_{ 1.8f };

		static VkVertexInputBindingDescription GetBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) {
			descriptions = {
				initInputBindingDesc(0, sizeof(PointLight), VK_VERTEX_INPUT_RATE_VERTEX)
			};
		}

		static void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
		{
			descriptions = {
				initInputAttrDesc(0, 0, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(PointLight, mPosition_)), // mPosition_
				initInputAttrDesc(0, 1, VK_FORMAT_R32_SFLOAT,  offsetof(PointLight, mConstant_)), // mConstant_
				initInputAttrDesc(0, 2, VK_FORMAT_R32_SFLOAT,  offsetof(PointLight, mLinear_)), // mLinear_
				initInputAttrDesc(0, 3, VK_FORMAT_R32_SFLOAT,  offsetof(PointLight, mQuadratic_)), // mQuadratic_
				initInputAttrDesc(0, 4, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(PointLight, mAmbient_)), // mAmbient_
				initInputAttrDesc(0, 5, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(PointLight, mDiffuse_)), // mDiffuse_
				initInputAttrDesc(0, 6, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(PointLight, mSpecular_)), // mSpecular_
			};
		}
	};

	class SpotLight
	{
	public:
		SpotLight() {}

		SpotLight(Vector3 Position, Vector3 Direction, Vector3 Ambient, Vector3 Diffuse, Vector3 Specular)
			: mPosition_(Position)
			, mDirection_(Direction.GetNormalized())
			, mAmbient_(Ambient)
			, mDiffuse_(Diffuse)
			, mSpecular_(Specular)
		{

		}

		alignas(16) Vector3 mPosition_;
		alignas(16) Vector3 mDirection_;
		
		alignas(16) Vector3 mAmbient_;
		alignas(16) Vector3 mDiffuse_;
		alignas(16) Vector3 mSpecular_;

		alignas(4) float mCutOff{ Cos(DegreeToRadian(15.f)) };
		alignas(4) float mOutCutoff{ Cos(DegreeToRadian(20.f)) };

		static VkVertexInputBindingDescription GetBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) {
			descriptions = {
				initInputBindingDesc(0, sizeof(SpotLight), VK_VERTEX_INPUT_RATE_VERTEX)
			};
		}

		static void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
		{
			descriptions = {
				initInputAttrDesc(0, 0, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(SpotLight, mPosition_)), // mPosition_
				initInputAttrDesc(0, 1, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(SpotLight, mDirection_)), // mDirection_
				initInputAttrDesc(0, 2, VK_FORMAT_R32_SFLOAT,  offsetof(SpotLight, mCutOff)), // mCutOff
				initInputAttrDesc(0, 3, VK_FORMAT_R32_SFLOAT,  offsetof(SpotLight, mOutCutoff)), // mQuadratic_
				initInputAttrDesc(0, 4, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(SpotLight, mAmbient_)), // mAmbient_
				initInputAttrDesc(0, 5, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(SpotLight, mDiffuse_)), // mDiffuse_
				initInputAttrDesc(0, 6, VK_FORMAT_R32G32B32_SFLOAT,  offsetof(SpotLight, mSpecular_)), // mSpecular_
			};
		}
	};

}