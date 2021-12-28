#include "VulkanInstance.h"


namespace zyh
{
	namespace VKHelper
	{
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) {
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else {
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr) {
				func(instance, debugMessenger, pAllocator);
			}
		}
	}

	void VulkanInstance::setup()
	{
		if (mEnableValidationLayers_ && !_checkValidationLayerSupport()) {
			tools::exitFatal("validation layers requested, but no available!");
		}

		_setupInstance();
		if (mEnableValidationLayers_)
			_setupDebugMessenger();
	}

	void VulkanInstance::cleanup()
	{
		if (mEnableValidationLayers_) {
			VKHelper::DestroyDebugUtilsMessengerEXT(mVkImpl_, mDebugMessenger_, nullptr);
		}
		vkDestroyInstance(mVkImpl_, nullptr);
	}

	const std::vector<const char*> VulkanInstance::_getRequiredExtensions()
	{
		std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME };
#if defined(_WIN32)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		if (mEnableValidationLayers_) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		if (!_checkExtensionsSupport(extensions.data(), static_cast<uint32_t>(extensions.size())))
			tools::exitFatal("extension requested, but no available!");
		return extensions;
	}

	const std::vector<const char*>& VulkanInstance::_getValidationLayers()
	{
		return mValidationLayers_;
	}

	bool VulkanInstance::_checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : _getValidationLayers())
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	bool VulkanInstance::_checkExtensionsSupport(const char** extensionNames, const uint32_t extensionCount)
	{
		for (uint32_t i = 0; i < extensionCount; ++i)
		{
			if (!_checkExtensionSupport(extensionNames[i]))
				return false;
		}
		return true;
	}

	bool VulkanInstance::_checkExtensionSupport(const char* extensionName)
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		bool extensionFound = false;
		for (const auto& extensionProperties : availableExtensions) {
			if (strcmp(extensionName, extensionProperties.extensionName) == 0) {
				extensionFound = true;
				break;
			}
		}
		return extensionFound;
	}

	void VulkanInstance::_populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}

	void VulkanInstance::_setupInstance()
	{
		// a struct with some information about our application.
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = Setting::AppTitle.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = Setting::EngineName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// tells the Vulkan driver which global extensions and validation layers we want to use.
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = _getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (mEnableValidationLayers_) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(_getValidationLayers().size());
			createInfo.ppEnabledLayerNames = _getValidationLayers().data();

			_populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &mVkImpl_), "failed to create instance!");
	}

	void VulkanInstance::_setupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		_populateDebugMessengerCreateInfo(createInfo);
		VK_CHECK_RESULT(VKHelper::CreateDebugUtilsMessengerEXT(mVkImpl_, &createInfo, nullptr, &mDebugMessenger_), "failed to set up debug messenger!");
	}
}


