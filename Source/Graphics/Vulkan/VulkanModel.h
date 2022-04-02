#pragma once
#include "Graphics/Common/IModel.h"
#include "IVulkanObject.h"

namespace zyh
{
	class VulkanModel : public IModel, public IVulkanObject
	{
	public:
		VulkanModel() : IModel()
		{
			mMesh_ = new VulkanMesh();
		}

	public:
		virtual IPrimitive* GeneratePrimitive()
		{
			return new VulkanPrimitives();
		}

	public:
		/// <summary>
		/// Implementation of IVulkanObject
		/// </summary>
		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool) 
		{
			static_cast<VulkanMesh*>(mMesh_)->connect(physicalDevice, logicalDevice, commandPool);
		}

		virtual void setup() override
		{
			static_cast<VulkanMesh*>(mMesh_)->setup();
		}

		virtual void cleanup() override
		{
			static_cast<VulkanMesh*>(mMesh_)->setup();
		}

	};
}