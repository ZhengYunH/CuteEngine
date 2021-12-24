#pragma once
#ifdef _WIN32
#include <windows.h>
#endif

#include "VulkanHeader.h"
#include "VulkanTools.h"
#include "Common/Setting.h"
#include "Common/KeyCodes.h"
#include "Camera/Camera.h"

namespace zyh
{
	class VulkanInstance;
	class VulkanSurface;
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanSwapchain;
	class VulkanCommandPool;
	class VulkanImage;
	class VulkanRenderPassBase;
	class VulkanGraphicsPipeline;

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};


	class VulkanBase
	{
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static VulkanBase* GVulkanInstance;
		static void registerInstance(VulkanBase* instance)
		{
			GVulkanInstance = instance;
		}

		

	public:
		VulkanBase() {};
		virtual ~VulkanBase();

		virtual void run(HINSTANCE hinstance)
		{
			registerInstance(this);
			initVulkan();
			initWindow(hinstance);
			setupVulkan();

			prepare();
			mainLoop();
			cleanup();
		}

	protected:
		virtual void initVulkan();
		virtual void initWindow(HINSTANCE hinstance)
		{
			setupWindow(hinstance, WndProc);
		}
		virtual void setupVulkan();
		virtual void prepare();
		virtual void mainLoop();
		virtual void cleanup();
		virtual void windowResize();

	protected: // Device Relate
		VkInstance mInstance_;
		VkDebugUtilsMessengerEXT mDebugMessenger_;

		/** @brief Encapsulated instance */
		VulkanInstance* mInstance_;

		VulkanSurface* mSurface_;

		/** @brief Encapsulated physical device */
		VulkanPhysicalDevice* mPhysicalDevice_; 

		/** @brief Encapsulated logical device */
		VulkanLogicalDevice* mLogicalDevice_;

		/** @brief Encapsulated swapchain*/
		VulkanSwapchain* mSwapchain_;

		/** @brief Encapsulated command pool*/
		VulkanCommandPool* mGraphicsCommandPool_;

		VulkanImage* mDepthStencil_;

		VulkanRenderPassBase* mRenderPass_;

		VulkanGraphicsPipeline* mGraphicsPipeline_;

		/** @brief Synchronization Objects*/
		const int MAX_FRAMES_IN_FLIGHT = 2;
		std::vector<VkSemaphore> mImageAvailableSemaphores_;
		std::vector<VkSemaphore> mRenderFinishedSemaphores_;
		std::vector<VkFence> mInFlightFences_;
		std::vector<VkFence> mImagesInFights_;

	protected: // Window Relate
#if defined(_WIN32)
		HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
		virtual void handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		HWND mWindow_;
		HINSTANCE mWindowInstance_;
#endif
		virtual VkSampleCountFlagBits getMsaaSamples();
		virtual VkFormat getDepthFormat();

	protected:
		bool mIsPaused_{ false };
		bool mIsResizing_{ false };
		bool mEnableValidationLayers_{ Setting::IsDebugMode };
		Camera mCamera_;

		TCache<VkSampleCountFlagBits> mMsaaSamples_;
		TCache<VkFormat> mDepthFromat_;

	private:
		const std::vector<const char*> mValidationLayers_ = {
			"VK_LAYER_KHRONOS_validation",
		};
		const std::vector<const char*> mDeviceExtensions_ = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

	private:
		void setupDepthStencil();
		// TODO: Encapsulated or insert to any class
		void createSyncObjects();

	// impl
	private:
		VulkanImage mColorResources_;
		void createColorResources();

		VulkanImage mDepthResources_;
		void createDepthResources();
	};

	VulkanBase* VulkanBase::GVulkanInstance = nullptr;
}
