#pragma once
#include "Graphics/Common/Geometry.h"
#include "IRenderScene.h"
#include "IMaterial.h"
#include "IRenderElement.h"

namespace zyh
{
	class IPrimitive
	{
	public:
		IPrimitive() { GenerateRenderElement(); }
		
		virtual ~IPrimitive() { SafeDestroy(mMaterial_); }

		// sent element to render scene
		virtual void CollectPrimitives(IRenderScene* renderScene); 

	protected:
		virtual void GenerateRenderElement() = 0;

	public:
		IMaterial* mMaterial_;
		IRenderElement* mRenderElement_;

		std::vector<Vertex> mVertices_;
		std::vector<uint32_t> mIndices_;
	};

	void IPrimitive::CollectPrimitives(IRenderScene* renderScene)
	{
		const TRenderSets& renderSets = mMaterial_->GetSupportRenderSet();
		for (auto& renderSet : renderSets)
		{
			renderScene->AddRenderElement(renderSet, mRenderElement_);
		}
	}
}