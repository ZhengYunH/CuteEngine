#pragma once
#include "Common/Config.h"
#include "Graphics/Vulkan/VulkanTools.h"
#include "Math/Matrix4x3.h"

namespace zyh
{
	class DirectionLight
	{
	public:
		DirectionLight() {}

		DirectionLight(Vector3 Direction, Vector3 Ambient, Vector3 Diffuse, Vector3 Specular)
			: mDirection_(Direction)
			, mAmbient_(Ambient)
			, mDiffuse_(Diffuse)
			, mSpecular_(Specular)
		{

		}

		alignas(16) Vector3 mDirection_;
		alignas(16) Vector3 mAmbient_;
		alignas(16) Vector3 mDiffuse_;
		alignas(16) Vector3 mSpecular_;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(DirectionLight);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions(uint32_t binding) {
			std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
			attributeDescriptions[0].binding = binding;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(DirectionLight, mDirection_);

			attributeDescriptions[1].binding = binding;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(DirectionLight, mAmbient_);

			attributeDescriptions[2].binding = binding;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(DirectionLight, mDiffuse_);

			attributeDescriptions[3].binding = binding;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(DirectionLight, mSpecular_);
			return attributeDescriptions;
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


		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(PointLight);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions(uint32_t binding) {
			std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions;
			
			attributeDescriptions[0].binding = binding;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(PointLight, mPosition_);

			attributeDescriptions[1].binding = binding;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(PointLight, mConstant_);

			attributeDescriptions[2].binding = binding;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(PointLight, mLinear_);

			attributeDescriptions[3].binding = binding;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(PointLight, mQuadratic_);

			attributeDescriptions[4].binding = binding;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(PointLight, mAmbient_);

			attributeDescriptions[5].binding = binding;
			attributeDescriptions[5].location = 5;
			attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[5].offset = offsetof(PointLight, mDiffuse_);

			attributeDescriptions[6].binding = binding;
			attributeDescriptions[6].location = 6;
			attributeDescriptions[6].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[6].offset = offsetof(PointLight, mSpecular_);
			
			return attributeDescriptions;
		}
	};

	class SpotLight
	{
	public:
		SpotLight() {}

		SpotLight(Vector3 Position, Vector3 Direction, Vector3 Ambient, Vector3 Diffuse, Vector3 Specular)
			: mPosition_(Position)
			, mDirection_(Direction)
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

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(SpotLight);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions(uint32_t binding) {
			std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions;

			attributeDescriptions[0].binding = binding;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(SpotLight, mPosition_);

			attributeDescriptions[1].binding = binding;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(SpotLight, mDirection_);

			attributeDescriptions[2].binding = binding;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(SpotLight, mCutOff);

			attributeDescriptions[3].binding = binding;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(SpotLight, mOutCutoff);

			attributeDescriptions[4].binding = binding;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(PointLight, mAmbient_);

			attributeDescriptions[5].binding = binding;
			attributeDescriptions[5].location = 5;
			attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[5].offset = offsetof(PointLight, mDiffuse_);

			attributeDescriptions[6].binding = binding;
			attributeDescriptions[6].location = 6;
			attributeDescriptions[6].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[6].offset = offsetof(PointLight, mSpecular_);

			return attributeDescriptions;
		}
	};

}