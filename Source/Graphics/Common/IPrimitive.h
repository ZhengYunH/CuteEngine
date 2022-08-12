#pragma once
#include "Graphics/Common/Geometry.h"
#include "Graphics/Common/ResourceLoader.h"
#include "Math/Vector3.h"
#include "IMaterial.h"
#include "Math/Matrix4x4.h"
#include "Graphics/Common/IRenderElement.h"


namespace zyh
{
	namespace PrimitiveType
	{
		enum EPrimitiveType
		{
			NONE = 0,
			MESH = 1,
			BOX = 2,
			SPHERE = 3
		};
	}
	using EPrimitiveType = PrimitiveType::EPrimitiveType;

	class IPrimitive
	{
	public:
		IPrimitive()
		{
			mMaterial_ = new IMaterial("Resource/shaders/vert.spv", "Resource/shaders/frag.spv");
		}

		IPrimitive(IMaterial* material)
		{
			mMaterial_ = material;
		}

		virtual ~IPrimitive() 
		{ 
			SafeDestroy(mMaterial_); 
		}

	public:
		IMaterial* GetMaterial() 
		{ 
			return mMaterial_; 
		}

		void SetTransform(Matrix4x3& mat)
		{
			mTransform_ = mat;
		}

		Matrix4x3 GetTransform()
		{
			return mTransform_;
		}

		bool IsStatic() { return mIsStatic_; }

		virtual void LoadResourceFile(const std::string& InFileName) = 0;

		virtual void GetBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) = 0;
		virtual void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions) = 0;

		virtual void GetVerticesData(void** data, size_t& size) = 0;
		virtual void GetIndicesData(void** data, size_t& size) = 0;

		virtual uint32_t GetVerticeCount() = 0;
		virtual uint32_t GetIndicesCount() = 0;

		virtual void AddRenderElement(RenderSet renderSet, IRenderElement* element)
		{
			mRenderElements_[renderSet] = element;
		}

		IRenderElement* GetRenderElement(RenderSet renderSet)
		{
			return mRenderElements_.at(renderSet);
		}

		std::vector<IRenderElement*> GetAllRenderElement()
		{
			std::vector<IRenderElement*> Elements;
			for (auto& pair : mRenderElements_)
			{
				Elements.push_back(pair.second);
			}
			return Elements;
		}

		void ClearRenderElement()
		{
			for (auto& pair : mRenderElements_)
			{
				delete pair.second;
			}
			mRenderElements_.clear();
		}

	protected:
		IMaterial* mMaterial_;
		std::unordered_map<RenderSet, IRenderElement*> mRenderElements_;
		EPrimitiveType mType_ { EPrimitiveType::MESH };
		Matrix4x3 mTransform_;
		bool	mIsStatic_{ true };
	};
	
	template<typename TVertexStruct = Vertex>
	class TPrimitive : public IPrimitive
	{
	public:
		TPrimitive() : IPrimitive() {}
		TPrimitive(IMaterial* material) : IPrimitive(material) {}

	public:
		virtual void LoadResourceFile(const std::string& InFileName)
		{
		}

		virtual void AddVertex(TVertexStruct point)
		{
			mVertices_.push_back(point);
		}

		void AddVertex(std::initializer_list<TVertexStruct> points)
		{
			for (auto point : points)
				AddVertex(point);
		}

		virtual void AddIndex(uint32_t index)
		{
			mIndices_.push_back(index);
		}

		void AddIndex(std::initializer_list<uint32_t> index)
		{
			for (auto indice : index)
				AddIndex(indice);
		}

		virtual void GetVerticesData(void** data, size_t& size) override
		{
			*data = mVertices_.data();
			size = mVertices_.size() * sizeof(TVertexStruct);
		}

		virtual void GetIndicesData(void** data, size_t& size) override
		{
			*data = mIndices_.data();
			size = mIndices_.size() * sizeof(uint32_t);
		}

		virtual uint32_t GetVerticeCount() override
		{
			return static_cast<uint32_t>(mVertices_.size());
		}

		virtual uint32_t GetIndicesCount() override
		{
			return static_cast<uint32_t>(mIndices_.size());
		}

		virtual void GetBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) override
		{
			TVertexStruct::GetBindingDescriptions(descriptions);
		}

		virtual void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions) override
		{
			TVertexStruct::GetAttributeDescriptions(descriptions);
		}

	protected:
		std::vector<TVertexStruct> mVertices_;
		std::vector<uint32_t> mIndices_;
	};

	class Primitive : public TPrimitive<Vertex>
	{
	public:
		Primitive() = default;
		Primitive(IMaterial* material) : TPrimitive<Vertex>(material) {}

		virtual void LoadResourceFile(const std::string& InFileName)
		{
			ResourceLoader::loadModel(InFileName, mVertices_, mIndices_);
		}
	};

	class SpherePrimitive : public Primitive
	{
	public:
		SpherePrimitive() : Primitive()
		{
			Initialize();
		}

		SpherePrimitive(IMaterial* material) : Primitive(material)
		{
			Initialize();
		}

		void Initialize()
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