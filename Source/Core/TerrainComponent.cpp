#include "TerrainComponent.h"
#include "Math/MathUtil.h"


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
		memset(mHeightMapData_, 0, mDepthCount_ * mWidthCount_ * sizeof(float));
	}

	void HeightMap::ModifyHeight(float x, float y, float offset)
	{
		int x_i = int(Clamp(x, 0.0f, float(mWidthCount_)));
		int y_i = int(Clamp(y, 0.0f, float(mDepthCount_)));
		mHeightMapData_[y_i * mWidthCount_ + x_i] += offset;
	}
}