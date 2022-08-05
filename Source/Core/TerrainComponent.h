#pragma once
#include "Common/Config.h"
#include "Graphics/Vulkan/VulkanHeader.h"
#include "IPrimitivesComponent.h"
#include "Core/EventHelper.h"


namespace zyh
{
	struct TerrainVert
	{
		glm::vec3 pos;
		glm::vec3 normal;
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
				initInputAttrDesc(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(TerrainVert, normal)), // normal
				initInputAttrDesc(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(TerrainVert, uv))	// uv
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
		uint32_t mDepthCount_{ 0 };
		uint32_t mWidthCount_{ 0 };

		Event<void, uint32_t, uint32_t> DataChanged;
	};


	class HeightMapManipulator
	{
	public:
		static HeightMapManipulator* getInstance()
		{
			static HeightMapManipulator* instance;
			if (!instance)
			{
				instance = new HeightMapManipulator();
			}
			return instance;
		}

	public:
		HeightMapManipulator()
		{
			BindInputEvent(LeftMouseDown, *this, HeightMapManipulator::EventLeftMouseDown);
			BindInputEvent(LeftMouseUp, *this, HeightMapManipulator::EventLeftMouseUp);
			BindInputEvent(MouseMove, *this, HeightMapManipulator::EventMouseMove);
		}

		void SetTargetHeightMap(HeightMap* heightMap)
		{
			mHeightmap_ = heightMap;
		}

		void tick()
		{
			if (mHeightmap_ && mEnable_ && mTouching_)
			{
				mHeightmap_->ModifyHeight(x, y, modifyTerrainOffset * GEngine->GetDeltaTime());
			}
		}

		bool mEnable_{ false };
		float modifyTerrainOffset = 0.f;
		float modifyTerrainRange = 1.f;

		bool mTouching_{ false };
		uint32_t x{ 5 };
		uint32_t y{ 5 };

		HeightMap* mHeightmap_{ nullptr };

	protected: // Event Binding
		void EventLeftMouseDown(KEY_TYPE x, KEY_TYPE y) { x = x; y = y; mTouching_ = true; }
		void EventLeftMouseUp(KEY_TYPE x, KEY_TYPE y) { x = x; y = y; mTouching_ = false; }
		void EventMouseMove(KEY_TYPE x, KEY_TYPE y) { x = x; y = y; }
	};

	class HeightMapPrimitive : public TPrimitive<TerrainVert>
	{
		using Super = TPrimitive<TerrainVert>;
	public:
		HeightMapPrimitive(IMaterial* material, HeightMap* heightMap) 
			: Super(material)
			, mHeightMap_{ heightMap }
		{
			uint32_t depthCount = mHeightMap_->mDepthCount_;
			uint32_t widthCount = mHeightMap_->mWidthCount_;

			mHeightMap_->DataChanged.Bind(Bind(&HeightMapPrimitive::HeightMapDataChanged, this));

			for (size_t i = 0; i < depthCount; ++i)
			{
				for (size_t j = 0; j < widthCount; ++j)
				{
					float u = 0;
					float v = 0;

					float fx0 = j != 0 ? mHeightMap_->mHeightMapData_[i * widthCount + j - 1] : 0.f;
					float fx1 = j != widthCount - 1 ? mHeightMap_->mHeightMapData_[i * widthCount + j + 1] : 0.f;
					float fy0 = i != 0 ?  mHeightMap_->mHeightMapData_[(i - 1) * widthCount + j] : 0.f;
					float fy1 = i != depthCount - 1 ? mHeightMap_->mHeightMapData_[(i + 1) * widthCount + j] : 0.f;
					
					AddVertex(
						TerrainVert(
							glm::vec3(i, mHeightMap_->mHeightMapData_[i * widthCount + j], j),
							glm::vec3((fx0 - fx1) / 2* mHeightMap_->mTileAcc_, 1, (fy0 - fy1) / 2*mHeightMap_->mTileAcc_),
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
		
			// [DEBUG]
			HeightMapManipulator::getInstance()->SetTargetHeightMap(heightMap);
		}

		void HeightMapDataChanged(uint32_t x, uint32_t y)
		{
			uint32_t widthCount = mHeightMap_->mWidthCount_;
			uint32_t index = y * widthCount + x;

			float fx0 = y != 0 ? mHeightMap_->mHeightMapData_[x * widthCount + y - 1] : 0.f;
			float fx1 = y != widthCount - 1 ? mHeightMap_->mHeightMapData_[x * widthCount + y + 1] : 0.f;
			float fy0 = x != 0 ? mHeightMap_->mHeightMapData_[(x - 1) * widthCount + y] : 0.f;
			float fy1 = x != widthCount - 1 ? mHeightMap_->mHeightMapData_[(x + 1) * mHeightMap_->mWidthCount_ + y] : 0.f;

			mVertices_[index].pos.g = mHeightMap_->mHeightMapData_[index];
			mVertices_[index].normal.r = (fx0 - fx1) / 2 * mHeightMap_->mTileAcc_;
			mVertices_[index].normal.b = (fy0 - fy1) / 2 * mHeightMap_->mTileAcc_;

			DataChanged.BoardCast(this);
		}

		Event<void, HeightMapPrimitive*> DataChanged;
	protected:
		HeightMap* mHeightMap_;
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
			mHeightmapPrim_->DataChanged.Bind(Bind(&TerrainComponent::HeightMapDataChanged, this));
			mModel_->AddPrimitive(mHeightmapPrim_);
		}

		virtual ~TerrainComponent()
		{
			SafeDestroy(mHeightmapPrim_);
			SafeDestroy(mMaterial_);
			SafeDestroy(mHeightmap_);
		}

		void HeightMapDataChanged(HeightMapPrimitive*);

	protected:
		HeightMap* mHeightmap_{ nullptr };
		HeightMapPrimitive* mHeightmapPrim_{ nullptr };
		IMaterial* mMaterial_;
	};
}