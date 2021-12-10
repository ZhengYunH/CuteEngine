#pragma once
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Setting.h"
#include "KeyCodes.h"

#include "Camera/Camera.h"

namespace VKHelper {
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}
}


class VulkanBase
{
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static VulkanBase* S_Instance;
	static void RegisterInstance(VulkanBase* instance)
	{
		S_Instance = instance;
	}

public:
	virtual void run(HINSTANCE hinstance)
	{
		RegisterInstance(this);
		initVulkan();
		initWindow(hinstance);
		prepare();
		mainLoop();
		cleanup();
	}

protected:
	virtual void initVulkan()
	{
	}

	virtual void initWindow(HINSTANCE hinstance)
	{
		setupWindow(hinstance, WndProc);
	}


	virtual void prepare()
	{
	}

	virtual void mainLoop() 
	{
	}

	virtual void cleanup()
	{
	}

	virtual void windowResize()
	{
	}

protected: // Device Relate
	VkInstance mInstance_;
	VkDebugUtilsMessengerEXT mDebugMessenger_;


protected: // Window Relate
#if defined(_WIN32)
	HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
	virtual void handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND mWindow_;
	HINSTANCE mWindowInstance_;
#endif

protected:
	bool mIsPaused_{ false };
	bool mIsResizing_{ false };
	bool mEnableValidationLayers_{ Setting::IsDebugMode };
	Camera mCamera_;


private:
	const std::vector<const char*> mValidationLayers_ = {
		"VK_LAYER_KHRONOS_validation",
	};
	const std::vector<const char*> mDeviceExtensions_ = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();

private:
	// Helper Function
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
};

VulkanBase*  VulkanBase::S_Instance = nullptr;