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
	public:
		IPrimitivesComponent(IEntity* Parent) : IComponent(Parent) {}
		IPrimitivesComponent(IEntity* Parent, EPrimitiveType meshType, const std::string& meshFileName) : IComponent(Parent)
		{
			GEngine->Scene->AddPrimitive(this);
			mModel_ = new VulkanModel(meshType, meshFileName);
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

	protected:
		VulkanModel* mModel_;
	};
}