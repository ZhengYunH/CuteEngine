#pragma once
#include "VulkanHeader.h"
#include "VulkanTools.h"
#include "Common/Setting.h"
#include "Common/KeyCodes.h"


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
	class VulkanRenderPass;
	class VulkanGraphicsPipeline;
	class VulkanBuffer;

	extern class VulkanBase* GVulkanInstance;

	class IRenderPass;

	class VulkanBase
	{
		static void registerInstance(VulkanBase* instance)
		{
			GVulkanInstance = instance;
		}

	public:
		VulkanBase() {};
		virtual ~VulkanBase();

		virtual void Initialize()
		{
			registerInstance(this);
			initVulkan();
			setupVulkan();
			prepare();
		}

	public:
		virtual void initVulkan();
		virtual void setupVulkan();
		virtual void prepare();
		virtual void CleanUp();
		virtual void windowResize(uint32_t width, uint32_t height);

	public: // Device Relate
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

		VulkanRenderPass* mFrameBufferRenderPass_{ nullptr };

		/** @brief Synchronization Objects*/
		const int MAX_FRAMES_IN_FLIGHT = 2;
		std::vector<VkSemaphore> mImageAvailableSemaphores_;
		std::vector<VkSemaphore> mRenderFinishedSemaphores_;
		std::vector<VkFence> mInFlightFences_;
		std::vector<VkFence> mImagesInFights_;

	protected:
		virtual VkSampleCountFlagBits getMsaaSamples();
		virtual VkFormat getDepthFormat();

	protected:
		bool mIsPaused_{ false };
		bool mIsResizing_{ false };
		uint32_t mWidth_{ 0 };
		uint32_t mHeight_{ 0 };
		bool mEnableValidationLayers_{ Setting::IsDebugMode };

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
		// TODO: Encapsulated or insert to any class
		void createSyncObjects();

	// impl
	private:
		VulkanImage* mColorResources_;
		void createColorResources();

		VulkanImage* mDepthResources_;
		void createDepthResources();

		std::vector<std::vector<VulkanCommand*>> mCommandBuffers_;
		void createCommandBuffers();
		VulkanCommand* createCommandBuffer(VkCommandBufferAllocateInfo* pAllocInfo = nullptr);
		void bindCommandBuffer();

		size_t mEncodeImage_ = 0;
		size_t mCurrentImage_ = 0;
		size_t mCurrentFrame_ = 0;
		bool mFrameBufferResized_ = false;
		virtual void drawFrame();
		virtual void recreateSwapchain();
		virtual void _cleanupSwapchain();

	public:
		void DrawFrameBegin(size_t& OutCurrentImage);
		void DrawFrameEnd() { drawFrame(); }
		size_t GetCurrentImage() { return mCurrentImage_; }
		VulkanCommand* GetCommandBuffer();
		VkFramebuffer GetSwapchainFrameBuffer();
		size_t mFreeCommandBufferIdx_{ 0 };
		uint32_t GetScreenHeigth() { return mHeight_; }
		uint32_t GetScreenWidth() { return mWidth_; }
	};
}
