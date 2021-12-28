#include <assert.h>
#include <string>
#include <iostream>

#include "VulkanTools.h"


namespace zyh
{
	namespace tools
	{
		bool errorModeSilent = false;

		LPCWSTR stringToLPCWSTR(const std::string& orig)
		{
			size_t origsize = orig.length() + 1;
			const size_t newsize = 100;
			size_t convertedChars = 0;
			wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
			mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

			return wcstring;
		}

		void exitFatal(int32_t exitCode)
		{
			exitFatal("Unknown Error", exitCode);
		}

		void exitFatal(int32_t exitCode, const std::string& message)
		{
			exitFatal(message, exitCode);
		}
		
		void exitFatal(const std::string& message, int32_t exitCode)
		{
#if defined(_WIN32)
			if (!errorModeSilent) {
				MessageBox(NULL, stringToLPCWSTR(message), NULL, MB_OK | MB_ICONERROR);
			}
#elif defined(__ANDROID__)
			LOGE("Fatal error: %s", message.c_str());
			vks::android::showAlert(message.c_str());
#endif
			std::cerr << message << "\n";
#if !defined(__ANDROID__)
			exit(exitCode);
#endif
		}
	}
}