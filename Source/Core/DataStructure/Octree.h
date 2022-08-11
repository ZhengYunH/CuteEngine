#pragma once
#include "Common/Config.h"
#include "Math/Box.h"


namespace zyh
{
	// xz-plane, y upward
	enum class EOctreeDirection : uint8_t
	{
		XP_YP_ZP = 0, // x+, y+, z+
		XN_YP_ZP = 1,
		XN_YP_ZN = 2,
		XP_YP_ZN = 3,
		XP_YN_ZP = 4,
		XN_YN_ZP = 5,
		XN_YN_ZN = 6,
		XP_YN_ZN = 7,
		Num = 8
	};

	template<typename TData>
	class OctreeNode
	{
	public:
		OctreeNode() : mData_ {nullptr}
		{
		}

		OctreeNode(TData* data) : mData_{ data }
		{
		}

		TData* GetData()
		{
			return mData_;
		}

	protected:
		OctreeNode* mParent_ { nullptr };
		OctreeNode* mChildren_{ nullptr };
		Box mBoundingBox_;

	protected:
		TData* mData_;
	};

	template<typename TData>
	class Octree
	{
	public:
		Octree() = default;

		virtual void InsertNode(TData* data) {};
		virtual void SearchSuitableNode(TData* data) {};


	protected:
		OctreeNode<TData> mRoot_;

	};
}