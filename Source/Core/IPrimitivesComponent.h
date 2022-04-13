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
		IPrimitivesComponent() {}
		IPrimitivesComponent(const std::string& meshFileName)
		{
			GEngine->Scene->AddPrimitive(this);
			mModel_ = new VulkanModel(meshFileName);
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

	protected:
		VulkanModel* mModel_;
	};
}