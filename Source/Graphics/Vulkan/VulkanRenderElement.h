#pragma once
#include "Graphics/Common/IRenderElement.h"
#include "VulkanMesh.h"
#include "IVulkanObject.h"
#include "VulkanModel.h"

namespace zyh
{
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanCommandPool;

	class VulkanRenderElement : public IRenderElement, public IVulkanObject
	{
	public:
		VulkanRenderElement() : IRenderElement()
		{ 
		}
		VulkanRenderElement(const std::string& meshFileName) : IRenderElement() 
		{
			mModel_ = new VulkanModel();
			LoadModel(meshFileName);
		}
		~VulkanRenderElement() { cleanup(); }

	public:
		/// <summary>
		/// Implementation of IVulkanObject
		/// </summary>
		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool)
		{
			static_cast<VulkanModel*>(mModel_)->connect(physicalDevice, logicalDevice, commandPool);
		}

		virtual void setup() override
		{
			static_cast<VulkanModel*>(mModel_)->setup();
		}

		virtual void cleanup() override
		{
			static_cast<VulkanModel*>(mModel_)->cleanup();
		}

	public:
		// TODO: should be optimized, less pipeline change
		void draw(VkCommandBuffer commandBuffer)
		{
			static_cast<VulkanMesh*>(mModel_->mMesh_)->draw(commandBuffer);
		}

	};
}