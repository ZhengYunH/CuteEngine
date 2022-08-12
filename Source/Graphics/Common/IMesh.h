#pragma once
#include <vector>
#include <string>

#include "Common/Config.h"
#include "IPrimitive.h"
#include "Math/Matrix4x4.h"

namespace zyh
{
	class IMesh
	{
	public:
		IMesh()
		{
			
		}

		virtual ~IMesh() 
		{
			for (auto prim : mPrimitives_)
				SafeDestroy(prim);
			mPrimitives_.clear();
		}

	public:
		const std::string& GetName() const
		{ 
			return mName_; 
		}

		IPrimitive* LoadResourceFile(const std::string& InFileName)
		{
			IPrimitive* prim = new Primitive();
			prim->LoadResourceFile(InFileName);
			return prim;
		}

		virtual size_t AddPrimitive(EPrimitiveType primType, Matrix4x3* localTransform = nullptr)
		{
			IPrimitive* prim;
			size_t index = -1;
			switch (primType)
			{
			case EPrimitiveType::MESH:
				index = AddPrimitive(new Primitive(), localTransform);
				break;
			case EPrimitiveType::BOX:
				Unimplement(0);
				break;
			case EPrimitiveType::SPHERE:
				index = AddPrimitive(new SpherePrimitive(), localTransform);

				// Test XRay Stencil
				prim = GetPrimitive(index);
				prim->GetMaterial()->AddRenderSet(RenderSet::XRAY, "Resource/shaders/XRayStencilWriter.vert.spv", "Resource/shaders/XRayStencilWriter.frag.spv");
				prim->GetMaterial()->GetPipelineState().DepthStencil.StencilTestEnable = true;
				prim->GetMaterial()->GetPipelineState().DepthStencil.StencilState.CompareOp = ECompareOP::ALWAYS;
				prim->GetMaterial()->GetPipelineState().DepthStencil.StencilState.PassOp = DepthStencilState::EStencilOp::REPLACE;
				prim->GetMaterial()->GetPipelineState().DepthStencil.StencilState.Reference = 1;
				break;
			default:
				Unimplement(0);
				break;
			}
			return index;
		}

		virtual size_t AddPrimitive(IPrimitive* prim, Matrix4x3* localTransform=nullptr) 
		{ 
			mPrimitives_.push_back(prim); 
			if (localTransform)
				mPrimitivesTransform_.push_back(*localTransform);
			else
				mPrimitivesTransform_.push_back(Matrix4x3());
			return mPrimitives_.size() - 1;
		}

		virtual IPrimitive* GetPrimitive(size_t index)
		{
			HYBRID_CHECK(index < mPrimitives_.size());
			return mPrimitives_[index];
		}

		virtual std::vector<IPrimitive*>& GetPrimitives()
		{
			return mPrimitives_;
		}

		void GetPrimLocalTransform(size_t index, Matrix4x3& outMatrix)
		{
			HYBRID_CHECK(index < mPrimitivesTransform_.size());
			outMatrix = mPrimitivesTransform_[index];
		}

		void UpdateEntityTransform(Matrix4x3& mat) 
		{ 
			EntityTransform_ = mat;
			for (size_t index =0; index < mPrimitives_.size(); ++index)
			{
				IPrimitive* prim = mPrimitives_[index];
				prim->SetTransform(EntityTransform_); // *PrimitivesTransform_[index]);
			}
		}

	public:
		std::vector<IPrimitive*> mPrimitives_;
		std::vector<Matrix4x3> mPrimitivesTransform_;
		Matrix4x3 EntityTransform_;
		std::string mName_;
	};
}