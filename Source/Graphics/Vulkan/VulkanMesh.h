#pragma once
#include "Graphics/Common/IMesh.h"
#include "IVulkanObject.h"
#include "VulkanPrimitives.h"

namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;

	class VulkanMesh : public IMesh, public IVulkanObject
	{
	public:
		VulkanMesh()
		{
		}

		virtual ~VulkanMesh()
		{
			cleanup();
		}

		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool)
		{
			for (auto& prim : mPrimitives_)
				static_cast<VulkanPrimitives*>(prim)->connect(physicalDevice, logicalDevice, commandPool);
		}

		virtual void setup() override
		{
			for (auto& prim : mPrimitives_)
				static_cast<VulkanPrimitives*>(prim)->setup();
		}

		virtual void cleanup() override
		{
			for (auto& prim : mPrimitives_)
				static_cast<VulkanPrimitives*>(prim)->cleanup();
		}

		virtual size_t AddPrimitive(IPrimitive* prim, Matrix4x3* localTransform = nullptr) override
		{
			HYBRID_CHECK(dynamic_cast<VulkanPrimitives*>(prim));
			return IMesh::AddPrimitive(prim);
		}

	public:
		void draw(VkCommandBuffer commandBuffer)
		{
			for (auto& prim : mPrimitives_)
				static_cast<VulkanPrimitives*>(prim)->draw(commandBuffer);
		}
	};



}