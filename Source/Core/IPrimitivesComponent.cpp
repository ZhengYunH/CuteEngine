#include "IPrimitivesComponent.h"
#include "File/FileSystem.h"
#include "Graphics/Vulkan/VulkanModel.h"

namespace zyh
{
	IPrimitivesComponent::IPrimitivesComponent(IEntity* Parent): IComponent(Parent)
	{
		mName_ = "IPrimitivesComponent";
		GEngine->Scene->AddPrimitive(this);
		mModel_ = new VulkanModel();
	}

	IPrimitivesComponent::IPrimitivesComponent(IEntity* Parent, EPrimitiveType meshType, const std::string& meshFileName) : IPrimitivesComponent(Parent)
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

	IPrimitivesComponent::~IPrimitivesComponent()
	{
		GEngine->Scene->DelPrimitive(this);
		SafeDestroy(mModel_);
	}

	void IPrimitivesComponent::EmitRenderElements(RenderSet renderSet, IRenderScene& renderScene)
	{
		mModel_->EmitRenderElements(renderSet, renderScene);
	}

	void IPrimitivesComponent::UpdateTransform(Matrix4x3& mat)
	{
		mModel_->UpdateTransform(mat);
	}

	void IPrimitivesComponent::Serialize(Archive* ar)
	{
		Super::Serialize(ar);
		ar->AddItem("EPrimitiveType", int(mMeshType_));
		if (!mMeshFileName_.empty())
		{
			ar->AddItem("ResourcePath", mMeshFileName_);
		}
	}
}

