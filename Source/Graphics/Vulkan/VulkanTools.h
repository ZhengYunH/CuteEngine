#pragma once

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <algorithm>

#if defined(_WIN32)
#include <wtypes.h>
#endif

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <set>
#include <optional>
#include <array>
#include <algorithm>
#include <functional>

#include "Common/Config.h"

#ifdef ZYH_DEBUG
#define VK_CHECK_RESULT(R, ...) if(VkResult res = (R); res != VK_SUCCESS) { zyh::tools::exitFatal(res, ##__VA_ARGS__); }
#else
#define VK_CHECK_RESULT(R, ...)
#endif

namespace zyh
{
	class VulkanInstance;

	template<typename _Type>
	class TCache
	{
	public:
		TCache() : mInstance_{ new _Type() }
		{
		}
		TCache(const TCache<_Type>& rhs)
		{
			mInstance_ = new _Type();
			*mInstance_ = *(rhs.mInstance_);
			mIsValid_ = rhs.mIsValid_;
		}
		TCache<_Type>& operator=(const TCache<_Type>& rhs)
		{
			if (this == &rhs)
			{
				return *this;
			}
			delete mInstance_;
			mInstance_ = new _Type();
			*mInstance_ = *(rhs.mInstance_);
			mIsValid_ = rhs.mIsValid_;
			return *this;
		}
		virtual ~TCache()
		{
			SafeDestroy(mInstance_);
		}
		_Type* operator ->()
		{
			return mInstance_;
		}
		const _Type* operator ->() const
		{
			return mInstance_;
		}

		_Type& operator *()
		{
			return *mInstance_;
		}
		const _Type& operator *() const
		{
			return *mInstance_;
		}

		const bool IsValid() { return mIsValid_; }
		void IsValid(bool isValid) { mIsValid_ = isValid; }
	private:
		_Type* mInstance_{ nullptr };
		bool mIsValid_{ false };
	};

	namespace tools
	{
#if defined(_WIN32)
		LPCWSTR stringToLPCWSTR(const std::string& orig);
#endif
		void exitFatal(int32_t exitCode=-1);
		void exitFatal(int32_t exitCode, const std::string& message);
		void exitFatal(const std::string& message, int32_t exitCode=-1);
	}

	extern VulkanInstance* GInstance;
}