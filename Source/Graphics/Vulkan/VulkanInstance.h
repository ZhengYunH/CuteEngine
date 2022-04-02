//	--------------------------------------------------------------------
//	Encapsulated Vulkan Instance
//	--------------------------------------------------------------------*/

#pragma once
#include "VulkanObject.h"
#include "Common/Setting.h"


namespace zyh
{
	namespace VKHelper {
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	}

	class VulkanInstance : public TVulkanObject<VkInstance>
	{
	public:
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
		const bool enableValidationLayers() { return mEnableValidationLayers_; };
		void enableValidationLayers(bool enable) { mEnableValidationLayers_ = enable; }
		virtual std::vector<const char*> getValidationLayers() { return mEnableValidationLayers_ ? mValidationLayers_ : std::vector<const char*>(); }
		void setup() override;
		void cleanup() override;

	public:
		TCache<VkFormat> mColorFormat_;
		TCache<VkFormat> mDepthFormat_;
		TCache<VkExtent2D> mExtend_;
		TCache<VkSampleCountFlagBits> mMsaaSamples_;

	protected:
		const std::vector<const char*> _getRequiredExtensions();
		const std::vector<const char*>& _getValidationLayers();

	private:
		bool _checkValidationLayerSupport();
		bool _checkExtensionsSupport(const char** extensionNames, const uint32_t extensionCount);
		bool _checkExtensionSupport(const char* extensionName);
		void _populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		// setup helper
		void _setupInstance();
		void _setupDebugMessenger();

	
	private: // Members
		bool mEnableValidationLayers_{ true };
		const std::vector<const char*> mValidationLayers_ = {
			"VK_LAYER_KHRONOS_validation",
		};
		VkDebugUtilsMessengerEXT mDebugMessenger_;
	};

	VulkanInstance* GInstance;
}