#include "VulkanSurface.h"
#include "VulkanInstance.h"

namespace zyh
{
	void VulkanSurface::connect(VulkanInstance* instance)
	{
		mVulkanInstance_ = instance;
	}

	void VulkanSurface::setup(void* platformHandle, void* platformWindow)
	{
		VkResult err = VK_SUCCESS;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = (HINSTANCE)platformHandle;
		surfaceCreateInfo.hwnd = (HWND)platformWindow;
		err = vkCreateWin32SurfaceKHR(mVulkanInstance_->Get(), &surfaceCreateInfo, nullptr, &mVkImpl_);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.window = window;
		err = vkCreateAndroidSurfaceKHR(mVulkanInstance_->Get(), &surfaceCreateInfo, NULL, &mVkImpl_);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
		VkIOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
		surfaceCreateInfo.pNext = NULL;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.pView = view;
		err = vkCreateIOSSurfaceMVK(mVulkanInstance_->Get(), &surfaceCreateInfo, nullptr, &mVkImpl_);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
		VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
		surfaceCreateInfo.pNext = NULL;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.pView = view;
		err = vkCreateMacOSSurfaceMVK(mVulkanInstance_->Get(), &surfaceCreateInfo, NULL, &mVkImpl_);
#elif defined(_DIRECT2DISPLAY)
		createDirect2DisplaySurface(width, height);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
		VkDirectFBSurfaceCreateInfoEXT surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
		surfaceCreateInfo.dfb = dfb;
		surfaceCreateInfo.surface = window;
		err = vkCreateDirectFBSurfaceEXT(mVulkanInstance_->Get(), &surfaceCreateInfo, nullptr, &mVkImpl_);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.display = display;
		surfaceCreateInfo.surface = window;
		err = vkCreateWaylandSurfaceKHR(mVulkanInstance_->Get(), &surfaceCreateInfo, nullptr, &mVkImpl_);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.connection = connection;
		surfaceCreateInfo.window = window;
		err = vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &mVkImpl_);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
		VkHeadlessSurfaceCreateInfoEXT surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
		PFN_vkCreateHeadlessSurfaceEXT fpCreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateHeadlessSurfaceEXT");
		if (!fpCreateHeadlessSurfaceEXT) {
			vks::tools::exitFatal("Could not fetch function pointer for the headless extension!", -1);
		}
		err = fpCreateHeadlessSurfaceEXT(mVulkanInstance_->Get(), &surfaceCreateInfo, nullptr, &mVkImpl_);
#endif
		VK_CHECK_RESULT(err, "Could not create surface!");
	}

}