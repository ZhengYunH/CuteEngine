#pragma once
#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <optional>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Common/Include/Setting.h"
#include "Common/Include/KeyCodes.h"

#include "Camera/Include/Camera.h"

#include "VulkanDevice.h"



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


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> trasnferFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


class VulkanBase
{
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static VulkanBase* GVulkanInstance;
	static void RegisterInstance(VulkanBase* instance)
	{
		GVulkanInstance = instance;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
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
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
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
	VkPhysicalDevice mPhysicalDevice_{ VK_NULL_HANDLE };

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
	void pickPhysicalDevice();

private:
	// Helper Function
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	bool checkExtensionsSupport(const char** extensionNames, const uint32_t extensionCount);
	bool checkExtensionSupport(const char* extensionName);

	int rateDeviceSuitability(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};

VulkanBase*  VulkanBase::GVulkanInstance = nullptr;