#pragma once

#include <assert.h>

#if defined(_WIN32)
#include <wtypes.h>
#endif

#include "Common/Config.h"

#ifdef ZYH_DEBUG
#define HYBRID_CHECK(X) assert(X)
#define VK_CHECK_RESULT(f)

#else
#define HYBRID_CHECK(X)
#define VK_CHECK_RESULT(f)
#endif

namespace zyh
{
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

		void exitFatal(const std::string& message, int32_t exitCode);
	}
}