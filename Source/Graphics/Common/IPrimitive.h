#pragma once
#include "Graphics/Common/Geometry.h"
#include "Graphics/Common/ResourceLoader.h"
#include "Math/Vector3.h"
#include "IMaterial.h"

namespace zyh
{
	class IPrimitive
	{
	public:
		IPrimitive()
		{
			mMaterial_ = new IMaterial();
		}

		IPrimitive(const std::string& InPrimFileName)
		{
			mMaterial_ = new IMaterial();
			ResourceLoader::loadModel(InPrimFileName, mVertices_, mIndices_);
		}

		virtual ~IPrimitive() 
		{ 
			SafeDestroy(mMaterial_); 
		}

	public:
		const IMaterial& GetMaterial() 
		{ 
			return *mMaterial_; 
		}

		void AddVertex(Vertex point)
		{
			mVertices_.push_back(point);
		}

		void AddIndex(uint32_t index)
		{
			mIndices_.push_back(index);
		}

	protected:
		IMaterial* mMaterial_;

	public:
		std::vector<Vertex> mVertices_;
		std::vector<uint32_t> mIndices_;
	};

	class SpherePrimitive : IPrimitive
	{
	public:
		SpherePrimitive() 
			: IPrimitive()
		{
			float startU = 0;
			float endU = 2 * MATH_PI;

			float startV = 0;
			float endV = MATH_PI;

			const size_t division = mDivision_;
			const float stepU = (endU - startU) / division;
			const float stepV = (endV - startV) / division;

			for (size_t i = 0; i < division; ++i)
			{
				for (size_t j = 0; j < division; ++j)
				{
					float u = i * stepU + startU;
					float v = j * stepV + startV;
					float un = (i + 1 == division) ? endU : (i + 1) * stepU + startU;
					float vn = (j + 1 == division) ? endU : (j + 1) * stepV + startV;
					
					Vector3 p0 = _GetUnitSphereSurfacePoint(u, v) * mRadius_;
					Vector3 p1 = _GetUnitSphereSurfacePoint(u, vn) * mRadius_;
					Vector3 p2 = _GetUnitSphereSurfacePoint(un, v) * mRadius_;
					Vector3 p3 = _GetUnitSphereSurfacePoint(un, vn) * mRadius_;

				}
			}
		}

	protected:
		Vector3 _GetUnitSphereSurfacePoint(float u, float v)
		{
			return Vector3(Cos(u) * Sin(v), Cos(v), Sin(u) * Sin(v));
		}

	protected:
		size_t mDivision_{ 24 };
		float mRadius_{ 1.f };
	};
}