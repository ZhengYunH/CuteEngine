#pragma once
#include "VulkanTools.h"

namespace zyh
{
	template<typename _VkType>
	class TVulkanObject
	{
	public:
		TVulkanObject() {}
		operator _VkType() { return mVkImpl_; }	// support implicit convert
		_VkType& Get() { return mVkImpl_; }
		const _VkType& Get() const { return mVkImpl_; }
		virtual void connect() {};
		virtual void setup() {};

	protected:
		_VkType mVkImpl_;
	};
}