#include "TerrainComponent.h"
#include "Math/MathUtil.h"
#include "Graphics/Vulkan/VulkanRenderElement.h"
#include "Graphics/Vulkan/VulkanModel.h"

#include <time.h>

namespace zyh 
{
	void HeightMap::GenerateData()
	{
		if (mHeightMapData_)
		{
			SafeDestroy(mHeightMapData_);
		}

		mDepthCount_ = static_cast<uint32_t>(Ceil(mTileDepth_ / mTileAcc_));
		mWidthCount_ = static_cast<uint32_t>(Ceil(mTileWidth_ / mTileAcc_));
		mHeightMapData_ = new float[mDepthCount_ * mWidthCount_];
		
		srand((unsigned)time(NULL));
		float randInit = (rand() % 255) / 255.f;
		for (size_t i = 0; i < mDepthCount_; ++i)
		{
			for (size_t j = 0; j < mWidthCount_; ++j)
			{
				randInit += ((rand() % 255) - 127) / 255.f;
				mHeightMapData_[i * mWidthCount_ + j] = randInit;
			}
			randInit = mHeightMapData_[i * mWidthCount_];
		}

		// memset(mHeightMapData_, 0, mDepthCount_ * mWidthCount_ * sizeof(float));
	}

	void HeightMap::ModifyHeight(float x, float y, float offset)
	{
		uint32_t x_i = uint32_t(Clamp(x, 0.0f, float(mWidthCount_)));
		uint32_t y_i = uint32_t(Clamp(y, 0.0f, float(mDepthCount_)));
		mHeightMapData_[y_i * mWidthCount_ + x_i] += offset;

		DataChanged.BoardCast(x_i, y_i);
	}

	TerrainComponent::TerrainComponent(IEntity* Parent): IPrimitivesComponent(Parent)
	{
		mName_ = "TerrainComponent";

		mHeightmap_ = new HeightMap();
		mHeightmap_->GenerateData();

		mMaterial_ = new IMaterial("Resource/shaders/terrain.vert.spv", "Resource/shaders/terrain.frag.spv");
		mHeightmapPrim_ = new HeightMapPrimitive(mMaterial_, mHeightmap_);
		mHeightmapPrim_->DataChanged.Bind(Bind(&TerrainComponent::HeightMapDataChanged, this));

		mModel_->AddPrimitive(mHeightmapPrim_);
	}

	void TerrainComponent::HeightMapDataChanged(HeightMapPrimitive* prim)
	{
		std::vector<VulkanRenderElement*> Elements;
		mModel_->getAllRenderElements(Elements);
		for (VulkanRenderElement* element : Elements)
		{
			element->updateData(prim);
		}
	}

}