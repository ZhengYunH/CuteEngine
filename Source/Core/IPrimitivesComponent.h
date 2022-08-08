#pragma once
#include "Common/Config.h"
#include "IComponent.h"
#include "Engine.h"
#include "ClientScene.h"
#include "Graphics/Common/ResourceLoader.h"
#include "Graphics/Vulkan/VulkanModel.h"


namespace zyh
{
	class IRenderScene;

	class IPrimitivesComponent : public IComponent
	{
		using Super = IComponent;
	public:
		IPrimitivesComponent(IEntity* Parent) : IComponent(Parent) 
		{
			mName_ = "IPrimitivesComponent"; 
			GEngine->Scene->AddPrimitive(this);
			mModel_ = new VulkanModel();

		}
		IPrimitivesComponent(IEntity* Parent, EPrimitiveType meshType, const std::string& meshFileName) : IPrimitivesComponent(Parent)
		{
			mMeshType_ = meshType;
			mMeshFileName_ = meshFileName;
			switch (meshType)
			{
			case EPrimitiveType::MESH:
				HYBRID_CHECK(!meshFileName.empty());
				mModel_->LoadResourceFile(meshFileName);
				break;
			default:
				mModel_->AddPrimitive(meshType);
				break;
			};
		}
		virtual ~IPrimitivesComponent()
		{
			GEngine->Scene->DelPrimitive(this);
			SafeDestroy(mModel_);
		}

	public:
		virtual void EmitRenderElements(RenderSet renderSet, IRenderScene& renderScene)
		{
			mModel_->EmitRenderElements(renderSet, renderScene);
		}
		virtual bool Culling() { return true; }
		virtual void UpdateTransform(Matrix4x3& mat) override { mModel_->UpdateTransform(mat); }

		virtual void Serialize(Archive* ar);

	protected:
		VulkanModel* mModel_;

		EPrimitiveType mMeshType_{ EPrimitiveType::MESH };
		std::string mMeshFileName_;
	};
}