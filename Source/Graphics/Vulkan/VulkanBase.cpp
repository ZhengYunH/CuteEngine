#include "VulkanBase.h"
#include "VulkanTools.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanImage.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"

#include "Graphics/Common/ResourceLoader.h"

namespace zyh
{
	VulkanBase* VulkanBase::GVulkanInstance = nullptr;

#if defined(_WIN32)
	HWND VulkanBase::setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
	{
		this->mWindowInstance_ = hinstance;

		WNDCLASSEX wndClass;

		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = wndproc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hinstance;
		wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = tools::stringToLPCWSTR(Setting::AppTitle);
		wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

		if (!RegisterClassEx(&wndClass))
		{
			std::cout << "Could not register window class!\n";
			fflush(stdout);
			exit(1);
		}

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		if (Setting::IsFullscreen)
		{
			if ((Setting::AppWidth != (uint32_t)screenWidth) && (Setting::AppHeight != (uint32_t)screenHeight))
			{
				DEVMODE dmScreenSettings;
				memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmPelsWidth = Setting::AppWidth;
				dmScreenSettings.dmPelsHeight = Setting::AppHeight;
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				{
					if (MessageBox(NULL, L"Full Screen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					{
						Setting::IsFullscreen = false;
					}
					else
					{
						return nullptr;
					}
				}
				screenWidth = Setting::AppWidth;
				screenHeight = Setting::AppHeight;
			}
		}

		DWORD dwExStyle;
		DWORD dwStyle;

		if (Setting::IsFullscreen)
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}
		else
		{
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}

		RECT windowRect;
		windowRect.left = 0L;
		windowRect.top = 0L;
		windowRect.right = Setting::IsFullscreen ? (long)screenWidth : (long)Setting::AppWidth;
		windowRect.bottom = Setting::IsFullscreen ? (long)screenHeight : (long)Setting::AppHeight;

		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

		std::string windowTitle = Setting::AppTitle;
		mWindow_ = CreateWindowEx(0,
			tools::stringToLPCWSTR(windowTitle),
			tools::stringToLPCWSTR(windowTitle),
			dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0,
			0,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			hinstance,
			NULL);

		if (!Setting::IsFullscreen)
		{
			// Center on screen
			uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
			uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
			SetWindowPos(mWindow_, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}

		if (!mWindow_)
		{
			printf("Could not create window!\n");
			fflush(stdout);
			return nullptr;
		}

		ShowWindow(mWindow_, SW_SHOW);
		SetForegroundWindow(mWindow_);
		SetFocus(mWindow_);

		return mWindow_;
	}

	LRESULT VulkanBase::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (VulkanBase::GVulkanInstance)
			VulkanBase::GVulkanInstance->handleMessage(hWnd, uMsg, wParam, lParam);
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}

	void VulkanBase::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CLOSE:
			mIsPaused_ = true;
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case KEY_W:
			case KEY_S:
			case KEY_A:
			case KEY_D:
				mCamera_.handleInputKeyDown(uMsg);
				break;
			}

		case WM_KEYUP:
			switch (wParam)
			{
			case KEY_P:
				mIsPaused_ = !mIsPaused_;
				break;
			case KEY_ESCAPE:
				PostQuitMessage(0);
				break;

			case KEY_W:
			case KEY_S:
			case KEY_A:
			case KEY_D:
				mCamera_.handleInputKeyUp(uMsg);
				break;
			}

		case WM_LBUTTONDOWN:
			mCamera_.handleMouseButtonDown(LEFT);
			break;
		case WM_RBUTTONDOWN:
			mCamera_.handleMouseButtonDown(RIGHT);
			break;
		case WM_MBUTTONDOWN:
			mCamera_.handleMouseButtonDown(MID);
			break;
		case WM_LBUTTONUP:
			mCamera_.handleMouseButtonUp(LEFT);
			break;
		case WM_RBUTTONUP:
			mCamera_.handleMouseButtonUp(RIGHT);
			break;
		case WM_MBUTTONUP:
			mCamera_.handleMouseButtonUp(MID);
			break;
		case WM_MOUSEWHEEL:
			mCamera_.handleMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
			break;
		case WM_MOUSEMOVE:
			mCamera_.handleMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED) // minimized
			{
				mIsPaused_ = true; // just pause rendering when minimize screen
			}
			else
			{
				mIsPaused_ = false;
				if(mIsResizing_)
					windowResize(LOWORD(lParam), HIWORD(lParam));
			}
			break;
		case WM_ENTERSIZEMOVE:
			mIsResizing_ = true;
			break;
		case WM_EXITSIZEMOVE:
			mIsResizing_ = false;
			break;
		}
	}
#endif

	VulkanBase::~VulkanBase()
	{

	}

	void VulkanBase::initVulkan()
	{
		mInstance_ = new VulkanInstance();
		mSurface_ = new VulkanSurface();
		mPhysicalDevice_ = new VulkanPhysicalDevice();
		mLogicalDevice_ = new VulkanLogicalDevice();
		mSwapchain_ = new VulkanSwapchain();
		mGraphicsCommandPool_ = new VulkanCommandPool(GRAPHICS);
		mDepthStencil_ = new VulkanImage();
		mRenderPass_ = new VulkanRenderPassBase();
		mGraphicsPipeline_ = new VulkanGraphicsPipeline();

		// connect
		mSurface_->connect(mInstance_);
		mPhysicalDevice_->connect(mInstance_, mSurface_);
		mLogicalDevice_->connect(mInstance_, mPhysicalDevice_);
		mSwapchain_->connect(mInstance_, mPhysicalDevice_, mLogicalDevice_, mSurface_);
		mGraphicsCommandPool_->connect(mPhysicalDevice_, mLogicalDevice_, mSwapchain_);
		mDepthStencil_->connect(mPhysicalDevice_, mLogicalDevice_);
		mRenderPass_->connect(mLogicalDevice_);
		mGraphicsPipeline_->connect(mLogicalDevice_, mRenderPass_);
	}

	void VulkanBase::setupVulkan()
	{
		mWidth_ = Setting::AppWidth;
		mHeight_ = Setting::AppHeight;

		mInstance_->setup();

#if defined(VK_USE_PLATFORM_WIN32_KHR)		
		mSurface_->setup(mWindowInstance_, mWindow_);
#endif

		mPhysicalDevice_->setup();
		
		mLogicalDevice_->setup();

		mSwapchain_->setup(&mWidth_, &mHeight_);
		
		mGraphicsCommandPool_->setup();
		
		mDepthStencil_->setup(
			mWidth_, mHeight_, 1,
			getMsaaSamples(), getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
		);
		
		mRenderPass_->setup(mSwapchain_->getColorFormat(), getMsaaSamples(), getDepthFormat());

		mGraphicsPipeline_->setup(
			"Resource/shaders/vert.spv",
			"Resource/shaders/frag.spv",
			mSwapchain_->getExtend(),
			getMsaaSamples()
		);
	}

	void VulkanBase::createSyncObjects()
	{
		mImageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
		mImagesInFights_.resize(mSwapchain_->getImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK_RESULT(vkCreateSemaphore(mLogicalDevice_->Get(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores_[i]), "failed to create image available semaphores!");
			VK_CHECK_RESULT(vkCreateSemaphore(mLogicalDevice_->Get(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores_[i]), "failed to create render finished semaphores!");
			VK_CHECK_RESULT(vkCreateFence(mLogicalDevice_->Get(), &fenceInfo, nullptr, &mInFlightFences_[i]), "failed to create fence!");
		}
	}

	VkSampleCountFlagBits VulkanBase::getMsaaSamples()
	{
		if (!mMsaaSamples_.IsValid())
		{
			*mMsaaSamples_ = mPhysicalDevice_->getMaxUsableSampleCount();
			mMsaaSamples_.IsValid(true);
		}
		return *mMsaaSamples_;
	}

	VkFormat VulkanBase::getDepthFormat()
	{
		if (!mDepthFromat_.IsValid())
		{
			*mDepthFromat_ = mPhysicalDevice_->findSupportedFormat(
				{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);
			mDepthFromat_.IsValid(true);
		}
		return *mDepthFromat_;
	}

	/// impl
	void VulkanBase::prepare()
	{
		createColorResources();
		createDepthResources();

		std::vector<VkImageView> attachments = {
			mColorResources_->Get().view,
			mDepthResources_->Get().view,
		};
		mSwapchain_->setupFrameBuffer(*mRenderPass_, attachments);

		loadData();

		createCommandBuffers();
		createSyncObjects();
	}

	void VulkanBase::mainLoop()
	{
		bool quitMessageReceived = false;
		while (!quitMessageReceived) {
#if defined(_WIN32)
			MSG msg;
			while (!quitMessageReceived) {
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					if (msg.message == WM_QUIT) {
						return;
					}
				}

				if(!mIsPaused_)
					drawFrame();
			}
#endif
			
		}

		vkDeviceWaitIdle(mLogicalDevice_->Get());
	}

	void VulkanBase::cleanup()
	{
		vkDeviceWaitIdle(mLogicalDevice_->Get());

		for (auto& buffer : mCommandBuffers_)
			SafeDestroy(buffer);

		for(auto& buffer : mUniformBuffers_)
			SafeDestroy(buffer)

		SafeDestroy(mTextureImage_);
		SafeDestroy(mDepthResources_);
		SafeDestroy(mColorResources_);
		SafeDestroy(mIndexBuffer_);
		SafeDestroy(mVertexBuffer_);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(mLogicalDevice_->Get(), mRenderFinishedSemaphores_[i], nullptr);
			vkDestroySemaphore(mLogicalDevice_->Get(), mImageAvailableSemaphores_[i], nullptr);
			vkDestroyFence(mLogicalDevice_->Get(), mInFlightFences_[i], nullptr);
		}
		SafeDestroy(mGraphicsPipeline_);
		SafeDestroy(mRenderPass_);
		SafeDestroy(mDepthStencil_);
		SafeDestroy(mSwapchain_);
		SafeDestroy(mLogicalDevice_);
		SafeDestroy(mPhysicalDevice_);
		SafeDestroy(mSurface_);
		SafeDestroy(mInstance_);
	}

	void VulkanBase::windowResize(uint32_t width, uint32_t height)
	{
		mWidth_ = width;
		mHeight_ = height;
		recreateSwapchain();
	}

	void VulkanBase::createColorResources()
	{
		mColorResources_ = new VulkanImage();
		mColorResources_->connect(mPhysicalDevice_, mLogicalDevice_);
		mColorResources_->setup(
			mSwapchain_->getExtend().width, mSwapchain_->getExtend().height, 1,
			getMsaaSamples(), mSwapchain_->getColorFormat(), VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	void VulkanBase::createDepthResources()
	{
		mDepthResources_ = new VulkanImage();
		mDepthResources_->connect(mPhysicalDevice_, mLogicalDevice_);
		mDepthResources_->setup(
			mSwapchain_->getExtend().width, mSwapchain_->getExtend().height, 1,
			getMsaaSamples(), getDepthFormat(), VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
		);
	}

	void VulkanBase::createTextureImage()
	{
		mTextureImage_ = new VulkanTextureImage("Resource/textures/viking_room.png");
		mTextureImage_->connect(mPhysicalDevice_, mLogicalDevice_, mGraphicsCommandPool_);
		mTextureImage_->setup(VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
		);
	}

	void VulkanBase::loadData()
	{
		createTextureImage();

		ResourceLoader::loadModel("Resource/models/viking_room.obj", mVertices_, mIndices_);

		// temp buffer
		VulkanBuffer stagingBuffer;
		stagingBuffer.connect(mPhysicalDevice_, mLogicalDevice_);

		// create Vertex Buffer
		VkDeviceSize bufferSize = sizeof(mVertices_[0]) * mVertices_.size();
		mVertexBuffer_ = new VulkanBuffer();
		mVertexBuffer_->connect(mPhysicalDevice_, mLogicalDevice_);
		mVertexBuffer_->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		stagingBuffer.setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.setupData(mVertices_.data(), bufferSize);
		VulkanBuffer::copyBuffer(mGraphicsCommandPool_, stagingBuffer.Get().buffer, mVertexBuffer_->Get().buffer, bufferSize);

		// create Index Buffer
		bufferSize = sizeof(mIndices_[0]) * mIndices_.size();
		mIndexBuffer_ = new VulkanBuffer();
		mIndexBuffer_->connect(mPhysicalDevice_, mLogicalDevice_);
		mIndexBuffer_->setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		stagingBuffer.setup(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.setupData(mIndices_.data(), bufferSize);
		VulkanBuffer::copyBuffer(mGraphicsCommandPool_, stagingBuffer.Get().buffer, mIndexBuffer_->Get().buffer, bufferSize);

		createUniformBuffer();
		createDescriptorSets();
	}

	void VulkanBase::createUniformBuffer()
	{
		mUniformBuffers_.resize(mSwapchain_->getImageCount());
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		for (auto& uniformBuffer : mUniformBuffers_)
		{
			uniformBuffer = new VulkanBuffer();
			uniformBuffer->connect(mPhysicalDevice_, mLogicalDevice_);
			uniformBuffer->setup(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
	}

	void VulkanBase::updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};

		Matrix4x4 modelMat, viewMat, projMat;
		modelMat = Matrix4x4::GenerateRotate(Matrix4x4(), time * glm::radians(90.0f), Vector3(0.0f, 0.0f, 1.0f));
		viewMat = Matrix4x4::GenerateLookAt(Vector3(2.0f, 2.0f, 2.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));
		projMat = Matrix4x4::GeneratePerspective(glm::radians(45.0f), mSwapchain_->getExtend().width / (float)mSwapchain_->getExtend().height, 0.1f, 10.0f);

		ubo.model = modelMat;
		ubo.view = viewMat;
		ubo.proj = projMat;
		ubo.proj[1][1] *= -1;

		mUniformBuffers_[currentImage]->setupData(&ubo, sizeof(ubo));
	}

	void VulkanBase::createDescriptorSets()
	{
		// create Descriptor Pool
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(mSwapchain_->getImageCount());

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(mSwapchain_->getImageCount());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(mSwapchain_->getImageCount());

		VK_CHECK_RESULT(vkCreateDescriptorPool(mLogicalDevice_->Get(), &poolInfo, nullptr, &mDescriptorPool_), "failed to create descriptor pool!");

		// bind Descriptor Sets
		std::vector<VkDescriptorSetLayout> layouts(mSwapchain_->getImageCount(), mGraphicsPipeline_->getDescriptorSetLayout());
		
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool_;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapchain_->getImageCount());
		allocInfo.pSetLayouts = layouts.data();

		mDescriptorSets_.resize(mSwapchain_->getImageCount());
		VK_CHECK_RESULT(vkAllocateDescriptorSets(mLogicalDevice_->Get(), &allocInfo, mDescriptorSets_.data()), "failed to allocate descriptor sets!");

		for (size_t i = 0; i < mDescriptorSets_.size(); i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers_[i]->Get().buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = mTextureImage_->Get().view;
			imageInfo.sampler = mTextureImage_->getTextureSampler();

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = mDescriptorSets_[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = mDescriptorSets_[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(mLogicalDevice_->Get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

	}

	void VulkanBase::createCommandBuffers()
	{
		mCommandBuffers_.resize(mSwapchain_->getImageCount());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mGraphicsCommandPool_->Get();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (size_t i = 0; i < mCommandBuffers_.size(); ++i) 
		{
			mCommandBuffers_[i] = new VulkanCommand();
			mCommandBuffers_[i]->connect(mLogicalDevice_, mGraphicsCommandPool_);
			mCommandBuffers_[i]->setup(allocInfo);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			mCommandBuffers_[i]->begin(&beginInfo);

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = mRenderPass_->Get();
			renderPassInfo.framebuffer = mSwapchain_->getFrameBuffer(static_cast<uint32_t>(i));
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = mSwapchain_->getExtend();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			VkCommandBuffer vkCommandBuffer = mCommandBuffers_[i]->Get();

			vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline_->Get());

			VkBuffer vertexBuffers[] = { mVertexBuffer_->Get().buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(vkCommandBuffer, mIndexBuffer_->Get().buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline_->getPipelineLayout(), 0, 1, &mDescriptorSets_[i], 0, nullptr);
			vkCmdDrawIndexed(vkCommandBuffer, static_cast<uint32_t>(mIndices_.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(vkCommandBuffer);
			
			mCommandBuffers_[i]->end();

		}
	}

	void VulkanBase::drawFrame()
	{
		vkWaitForFences(mLogicalDevice_->Get(), 1, &mInFlightFences_[mCurrentFrame_], VK_TRUE, UINT64_MAX);

		// Acquiring an image from the swap chain
		uint32_t imageIndex;
		VkResult result = mSwapchain_->acquireNextImage(mImageAvailableSemaphores_[mCurrentFrame_], &imageIndex);

		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (mImagesInFights_[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(mLogicalDevice_->Get(), 1, &mImagesInFights_[imageIndex], VK_TRUE, UINT64_MAX);
		}
		// Mark the image as now being in use by this frame
		mImagesInFights_[imageIndex] = mImagesInFights_[mCurrentFrame_];

		updateUniformBuffer(imageIndex);

		// Submitting the command buffer
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores_[mCurrentFrame_] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers_[imageIndex]->Get();

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores_[mCurrentFrame_] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(mLogicalDevice_->Get(), 1, &mInFlightFences_[mCurrentFrame_]);
		VK_CHECK_RESULT(vkQueueSubmit(mLogicalDevice_->graphicsQueue(), 1, &submitInfo, mInFlightFences_[mCurrentFrame_]), "failed to submit draw command buffer!");

		// Presentation
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapchain_->Get() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(mLogicalDevice_->presentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFrameBufferResized_) {
			mFrameBufferResized_ = false;
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		mCurrentFrame_ = (mCurrentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanBase::recreateSwapchain()
	{
		// Ensure all operations on the device have been finished before destroying resources
		vkDeviceWaitIdle(mLogicalDevice_->Get());
		_cleanupSwapchain();

		// Recreate swap chain
		mSwapchain_->setup(&mWidth_, &mHeight_);
		mRenderPass_->setup(mSwapchain_->getColorFormat(), getMsaaSamples(), getDepthFormat());

		mGraphicsPipeline_->setup(
			"Resource/shaders/vert.spv",
			"Resource/shaders/frag.spv",
			mSwapchain_->getExtend(),
			getMsaaSamples()
		);
		createColorResources();
		createDepthResources();

		std::vector<VkImageView> attachments = {
			mColorResources_->Get().view,
			mDepthResources_->Get().view,
		};
		mSwapchain_->setupFrameBuffer(*mRenderPass_, attachments);
		
		createUniformBuffer();
		createDescriptorSets();
		createCommandBuffers();
	}

	void VulkanBase::_cleanupSwapchain()
	{
		mDepthStencil_->cleanup();
		mColorResources_->cleanup();
		
		for (auto& buffer : mCommandBuffers_)
			SafeDestroy(buffer);

		for (auto& buffer : mUniformBuffers_)
			SafeDestroy(buffer);
		
		mGraphicsPipeline_->cleanup();
		mRenderPass_->cleanup();
		mSwapchain_->cleanup();
		vkDestroyDescriptorPool(mLogicalDevice_->Get(), mDescriptorPool_, nullptr);
	}

}
