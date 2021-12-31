#pragma once
#if defined(_WIN32)
#include <windows.h>
#endif

#include "VulkanHeader.h"
#include "VulkanTools.h"
#include "Common/Setting.h"
#include "Common/KeyCodes.h"
#include "Camera/Camera.h"
#include "Graphics/Common/Geometry.h"


namespace zyh
{
	class VulkanInstance;
	class VulkanSurface;
	class VulkanPhysicalDevice;
	class VulkanLogicalDevice;
	class VulkanSwapchain;
	class VulkanCommandPool;
	class VulkanCommand;
	class VulkanImage;
	class VulkanTextureImage;
	class VulkanRenderPassBase;
	class VulkanGraphicsPipeline;
	class VulkanBuffer;


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
		virtual void windowResize(uint32_t width, uint32_t height);


	protected: // Device Relate
		/** @brief Encapsulated instance */
		VulkanInstance* mInstance_{ nullptr };
		VkDebugUtilsMessengerEXT mDebugMessenger_;

		VulkanSurface* mSurface_{ nullptr };

		/** @brief Encapsulated physical device */
		VulkanPhysicalDevice* mPhysicalDevice_{ nullptr };

		/** @brief Encapsulated logical device */
		VulkanLogicalDevice* mLogicalDevice_{ nullptr };

		/** @brief Encapsulated swapchain*/
		VulkanSwapchain* mSwapchain_{ nullptr };

		/** @brief Encapsulated command pool*/
		VulkanCommandPool* mGraphicsCommandPool_{ nullptr };

		VulkanImage* mDepthStencil_{ nullptr };

		VulkanRenderPassBase* mRenderPass_{ nullptr };

		VulkanGraphicsPipeline* mGraphicsPipeline_{ nullptr };

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
		uint32_t mWidth_{ 0 };
		uint32_t mHeight_{ 0 };
		bool mEnableValidationLayers_{ Setting::IsDebugMode };
		Camera mCamera_;
		// TODO
		float mDeltaTime_{ 0.033f };

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
		VulkanImage* mColorResources_;
		void createColorResources();

		VulkanImage* mDepthResources_;
		void createDepthResources();

		VulkanTextureImage* mTextureImage_;
		void createTextureImage();

		std::vector<Vertex> mVertices_;
		std::vector<uint32_t> mIndices_;
		VulkanBuffer* mVertexBuffer_;
		VulkanBuffer* mIndexBuffer_;
		struct UniformBufferObject {
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};
		std::vector<VulkanBuffer*> mUniformBuffers_;
		void loadData();
		void createUniformBuffer();
		void updateUniformBuffer(uint32_t currentImage);

		VkDescriptorPool mDescriptorPool_;
		std::vector<VkDescriptorSet> mDescriptorSets_;
		void createDescriptorSets();

		std::vector<VulkanCommand*> mCommandBuffers_;
		void createCommandBuffers();

		size_t mCurrentFrame_ = 0;
		bool mFrameBufferResized_ = false;
		virtual void drawFrame();
		virtual void recreateSwapchain();
		virtual void _cleanupSwapchain();
	};
}
