#pragma once
#include "VulkanTools.h"
#include "IVulkanObject.h"


namespace zyh
{
	template<typename _VkType>
	class TVulkanObject: IVulkanObject
	{
	public:
		TVulkanObject() {}
		operator _VkType() { return mVkImpl_; }	// support implicit convert
		_VkType& Get() { return mVkImpl_; }
		const _VkType& Get() const { return mVkImpl_; }
		virtual void connect() override {};
		virtual void setup() override {};
		virtual void cleanup() override {};

	protected:
		_VkType mVkImpl_;
	};
}