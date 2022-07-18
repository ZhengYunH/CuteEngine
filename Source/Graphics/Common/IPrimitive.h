#pragma once
#include "Graphics/Common/Geometry.h"
#include "Graphics/Common/ResourceLoader.h"
#include "Math/Vector3.h"
#include "IMaterial.h"
#include "Math/Matrix4x4.h"

namespace zyh
{
	enum EPrimitiveType
	{
		NONE = 0,
		MESH = 1,
		BOX = 2,
		SPHERE = 3
	};

	class IPrimitive
	{
	public:
		IPrimitive() : mType_(EPrimitiveType::MESH)
		{
			mMaterial_ = new IMaterial();
		}

		IPrimitive(const std::string& InPrimFileName) : mType_(EPrimitiveType::MESH)
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

		void SetTransform(Matrix4x3& mat)
		{
			mTransform_ = mat;
		}

		Matrix4x3 GetTransform()
		{
			return mTransform_;
		}

	protected:
		IMaterial* mMaterial_;
		EPrimitiveType mType_;
		Matrix4x3 mTransform_;

	public:
		std::vector<Vertex> mVertices_;
		std::vector<uint32_t> mIndices_;
	};

	class SpherePrimitive : public IPrimitive
	{
	public:
		SpherePrimitive() 
			: IPrimitive()
		{
			mType_ = EPrimitiveType::SPHERE;

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
					float vn = (j + 1 == division) ? endV : (j + 1) * stepV + startV;
					
					Vector3 p0 = _GetUnitSphereSurfacePoint(u, v) * mRadius_;
					Vector3 p1 = _GetUnitSphereSurfacePoint(u, vn) * mRadius_;
					Vector3 p2 = _GetUnitSphereSurfacePoint(un, v) * mRadius_;
					Vector3 p3 = _GetUnitSphereSurfacePoint(un, vn) * mRadius_;

					glm::vec2 texCoord(-1.f, -1.f);

					auto colorGenerate = [](float u, float v) ->  glm::vec3
					{ 
						return  glm::vec3((Sin(u) + 1) * 0.5, (Cos(v) + 1) * 0.5, 1.f); 
					}; 

					Vertex v0(glm::vec3(p0.x, p0.y, p0.z), colorGenerate(u, v), texCoord);
					Vertex v1(glm::vec3(p1.x, p1.y, p1.z), colorGenerate(u, vn), texCoord);
					Vertex v2(glm::vec3(p2.x, p2.y, p2.z), colorGenerate(un, v), texCoord);
					Vertex v3(glm::vec3(p3.x, p3.y, p3.z), colorGenerate(un, vn), texCoord);

					uint32_t index = static_cast<uint32_t>(mVertices_.size());
					AddVertex(v0);
					AddVertex(v1);
					AddVertex(v2);
					AddVertex(v3);

					AddIndex(index + 0);
					AddIndex(index + 1);
					AddIndex(index + 2);

					AddIndex(index + 1);
					AddIndex(index + 3);
					AddIndex(index + 2);
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