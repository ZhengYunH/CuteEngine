#pragma once
#include "Graphics/Common/IModel.h"
#include "Graphics/Common/IRenderScene.h"
#include "Graphics/Common/IMaterial.h"
#include "IVulkanObject.h"
#include "VulkanRenderElement.h"

namespace zyh
{
	class VulkanModel : public IVulkanObject, public IModel
	{
	public:
		VulkanModel() : IModel()
		{
		}

	public: // override
		virtual size_t AddPrimitive(IPrimitive* prim, Matrix4x3* localTransform = nullptr) override
		{
			size_t index = IModel::AddPrimitive(prim, localTransform);
			if (prim->IsStatic())
			{
				GenerateRenderElement(prim);
			}
			return index;
		}

		virtual size_t AddPrimitive(EPrimitiveType primType, Matrix4x3* localTransform = nullptr) override
		{
			size_t index = IModel::AddPrimitive(primType, localTransform);
			IPrimitive* prim = mMesh_->GetPrimitive(index);
			if (prim->IsStatic())
			{
				GenerateRenderElement(prim);
			}
			return index;
		}
		
		
		virtual void UpdateTransform(Matrix4x3& mat) 
		{ 
			IModel::UpdateTransform(mat);
			std::vector<VulkanRenderElement*> allElements;
			getAllRenderElements(allElements);
			for (VulkanRenderElement* element : allElements)
			{
				element->UpdateTransform(mat);
			}
		}


	public:
		void EmitRenderElements(RenderSet renderSet, IRenderScene& renderScene)
		{
			auto iter = mRenderElements_.find(renderSet);
			if (iter != mRenderElements_.end())
			{
				std::vector<IRenderElement*>& renderElements = iter->second;
				for (auto element : renderElements)
				{
					renderScene.AddRenderElement(renderSet, element);
				}
			}
		}
		
	protected:
		void GenerateRenderElement(IPrimitive* prim)
		{
			const IMaterial* material = prim->GetMaterial();
			for (RenderSet renderSet : material->GetSupportRenderSet())
			{
				if (mRenderElements_.find(renderSet) == mRenderElements_.end())
				{
					mRenderElements_[renderSet] = *(new std::vector<IRenderElement*>());
				}
				IRenderElement* element = new VulkanRenderElement(prim, renderSet);
				prim->AddRenderElement(renderSet, element);
			}
		}

		void GenerateRenderElements()
		{
			auto& prims = GetPrimitives();
			for (auto& prim : prims)
			{
				prim->ClearRenderElement();
				GenerateRenderElement(prim);
			}
		}

	public:
		/// <summary>
		/// Implementation of IVulkanObject
		/// </summary>
		virtual void connect(VulkanPhysicalDevice* physicalDevice, VulkanLogicalDevice* logicalDevice, VulkanCommandPool* commandPool) 
		{
			mVulkanPhysicalDevice_ = physicalDevice;
			mVulkanLogicalDevice_ = logicalDevice;
			mVulkanCommandPool_ = commandPool;
			
			std::vector<VulkanRenderElement*> allElements;
			getAllRenderElements(allElements);
			for (VulkanRenderElement* element : allElements)
			{
				element->connect(physicalDevice, logicalDevice, commandPool);
			}
		}

		virtual void setup() override
		{
			std::vector<VulkanRenderElement*> allElements;
			getAllRenderElements(allElements);
			for (VulkanRenderElement* element : allElements)
			{
				element->setup();
			}
		}

		virtual void cleanup() override
		{
			std::vector<VulkanRenderElement*> allElements;
			getAllRenderElements(allElements);
			for (VulkanRenderElement* element : allElements)
			{
				element->cleanup();
				SafeDestroy(element);
			}
			mRenderElements_.clear();
		}

	public:
		void getAllRenderElements(std::vector<VulkanRenderElement*>& OutElements)
		{
			OutElements.clear();
			auto& prims = GetPrimitives();
			for (auto prim : prims)
			{
				for (auto element : prim->GetAllRenderElement())
				{
					OutElements.push_back(static_cast<VulkanRenderElement*>(element));
				}
			}
		}

	protected:
		VulkanPhysicalDevice* mVulkanPhysicalDevice_;
		VulkanLogicalDevice* mVulkanLogicalDevice_;
		VulkanCommandPool* mVulkanCommandPool_;

	protected:
		IModel* mModel_;
		std::map<RenderSet, std::vector<IRenderElement*>> mRenderElements_;
	};
}