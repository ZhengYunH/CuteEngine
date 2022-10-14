#pragma once
#include "Common/Config.h"
#include "IComponent.h"
#include "Engine.h"
#include "ClientScene.h"
#include "Graphics/Common/ResourceLoader.h"
#include "Graphics/Common/IPrimitive.h"


namespace zyh
{
	class IRenderScene;
	class VulkanModel;

	class IPrimitivesComponent : public IComponent
	{
		using Super = IComponent;
	public:
		IPrimitivesComponent(IEntity* Parent);
		IPrimitivesComponent(IEntity* Parent, EPrimitiveType meshType, const std::string& meshFileName);
		virtual ~IPrimitivesComponent();

	public:
		virtual void EmitRenderElements(RenderSet renderSet, IRenderScene& renderScene);
		virtual bool Culling() { return true; }
		virtual void UpdateTransform(Matrix4x3& mat) override;
		virtual void Serialize(Archive* ar);

	protected:
		VulkanModel* mModel_;

		EPrimitiveType mMeshType_{ EPrimitiveType::MESH };
		std::string mMeshFileName_;
	};
}