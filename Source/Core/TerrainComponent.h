#pragma once
#include "Common/Config.h"
#include "Graphics/Vulkan/VulkanHeader.h"
#include "IPrimitivesComponent.h"


namespace zyh
{
	struct TerrainVert
	{
		glm::vec3 pos;
		glm::vec2 uv;

		static void GetBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions){
			descriptions = {
				initInputBindingDesc(0, sizeof(TerrainVert), VK_VERTEX_INPUT_RATE_VERTEX)
			};
		}

		static void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
		{
			descriptions = {
				initInputAttrDesc(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(TerrainVert, pos)), // pos
				initInputAttrDesc(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(TerrainVert, uv))	// uv
			};
		}
	};

	struct HeightMap
	{
		HeightMap(){}
		void GenerateData();
		void ModifyHeight(float x, float y, float offset);

		// setting
		uint32_t mTileWidth_{ 256 };
		uint32_t mTileDepth_{ 256 };
		float mTileAcc_{ 1.0f };

		// data
		float* mHeightMapData_{ nullptr };
		uint32_t mDepthCount_;
		uint32_t mWidthCount_;
	};

	class HeightMapPrimitive : public TPrimitive<TerrainVert>
	{
		using Super = TPrimitive<TerrainVert>;
	public:
		HeightMapPrimitive(IMaterial* material, HeightMap* sourceData) 
			: Super(material)
			, mHeightMap_{ sourceData }
		{
			uint32_t depthCount = mHeightMap_->mDepthCount_;
			uint32_t widthCount = mHeightMap_->mWidthCount_;

			for (size_t i = 0; i < depthCount; ++i)
			{
				for (size_t j = 0; j < widthCount; ++j)
				{
					float u = 0;
					float v = 0;
					AddVertex(
						TerrainVert(
							glm::vec3(i, mHeightMap_->mHeightMapData_[i * widthCount + j], j),
							glm::vec2(u, v)
							)
					);
					
				}
			}

			for (uint32_t i = 0; i < depthCount - 1; ++i)
			{
				for (uint32_t j = 0; j < widthCount - 1; ++j)
				{

					/*
						  -      ---
						 --      --
						---      -
					*/
					AddIndex(i * widthCount + j);
					AddIndex(i * widthCount + j + 1);
					AddIndex((i + 1) * widthCount + j + 1);

					AddIndex(i * widthCount + j);
					AddIndex((i + 1) * widthCount + j + 1);
					AddIndex((i + 1) * widthCount + j);
				}
			}
		}

	protected:
		HeightMap* mHeightMap_;
	};

	class HeightMapManipulator
	{
	public:
		HeightMapManipulator(HeightMap* heightMap) : mHeightmap_{ heightMap }
		{

		}

		bool mEnable_{ false };
		HeightMap* mHeightmap_{ nullptr };
	};

	class TerrainComponent : public IPrimitivesComponent
	{
		using Super = IComponent;
	public:
		TerrainComponent(IEntity* Parent) : IPrimitivesComponent(Parent)
		{ 
			mName_ = "TerrainComponent"; 

			mHeightmap_ = new HeightMap();
			mHeightmap_->GenerateData();

			mMaterial_ = new IMaterial("Resource/shaders/terrain.vert.spv", "Resource/shaders/terrain.frag.spv");
			mHeightmapPrim_ = new HeightMapPrimitive(mMaterial_, mHeightmap_);
			mModel_->AddPrimitive(mHeightmapPrim_);
		}

		virtual ~TerrainComponent()
		{
			SafeDestroy(mHeightmap_);
		}

	protected:
		HeightMap* mHeightmap_{ nullptr };
		HeightMapPrimitive* mHeightmapPrim_{ nullptr };
		IMaterial* mMaterial_;
	};
}