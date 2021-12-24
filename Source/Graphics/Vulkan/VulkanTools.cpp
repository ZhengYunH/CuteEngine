#include <assert.h>
#include <string>
#include <iostream>
#include <WinUser.h>

#include "VulkanTools.h"


namespace zyh
{
	namespace tools
	{
		bool errorModeSilent = false;

		void exitFatal()
		{
			exitFatal("Unknown Error", -1);
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