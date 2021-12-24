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
#define VK_CHECK_RESULT(R, ...) if((R) != VK_SUCCESS) { zyh::tools::exitFatal(__VA_ARGS__); }
#else
#define VK_CHECK_RESULT(R, ...)
#endif

namespace zyh
{
	//template<typename _Type>
	//inline _Type& max(const _Type& v1, const _Type& v2) 
	//{
	//	return v1 > v2 ? v1 : v2;
	//}

	template<typename _Type>
	class TCache
	{
	public:
		TCache() : mInstance_{ new _Type() }
		{

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
		bool mIsValid_{ true };
	};

	namespace tools
	{
#if defined(_WIN32)
		LPCWSTR stringToLPCWSTR(const std::string& orig)
		{
			size_t origsize = orig.length() + 1;
			const size_t newsize = 100;
			size_t convertedChars = 0;
			wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
			mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

			return wcstring;
		}
#endif
		void exitFatal();
		void exitFatal(const std::string& message, int32_t exitCode=-1);
	}
}