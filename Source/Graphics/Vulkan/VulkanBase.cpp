#include "VulkanBase.h"
#include "VulkanTools.h"

#ifdef _WIN32
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
	wndClass.lpszClassName = stringToLPCWSTR(Setting::AppTitle);
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
		stringToLPCWSTR(windowTitle),
		stringToLPCWSTR(windowTitle),
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
	if(VulkanBase::GVulkanInstance)
		VulkanBase::GVulkanInstance->handleMessage(hWnd, uMsg, wParam, lParam);
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void VulkanBase::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
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
		short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		mCamera_.handleMouseWheel(wheelDelta);
		break;
	case WM_MOUSEMOVE:
		mCamera_.handleMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_SIZE:
		windowResize();
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

void VulkanBase::createInstance()
{
	if (mEnableValidationLayers_ && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but no available!");
	}

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

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (mEnableValidationLayers_) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers_.size());
		createInfo.ppEnabledLayerNames = mValidationLayers_.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// the 3rd parameter(@instance) stores the handle to the new object
	if (vkCreateInstance(&createInfo, nullptr, &mInstance_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanBase::setupDebugMessenger()
{
	if (!mEnableValidationLayers_)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	if (VKHelper::CreateDebugUtilsMessengerEXT(mInstance_, &createInfo, nullptr, &mDebugMessenger_) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void VulkanBase::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance_, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance_, &deviceCount, devices.data());


	uint8_t getPhysicsDeviceMethod = 1;
	if (getPhysicsDeviceMethod == 0) // get highest score physics device
	{
		// Use an ordered map to automatically sort candidates by increasing score
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : devices) {
			int score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}
		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0) {
			mPhysicalDevice_ = candidates.rbegin()->second;
		}
	}
	else {
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				mPhysicalDevice_ = device;
				mMsaaSamples_ = getMaxUsableSampleCount();
				break;
			}
		}
	}
}

bool VulkanBase::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : mValidationLayers_)
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

std::vector<const char*> VulkanBase::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (mEnableValidationLayers_) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	if (!checkExtensionsSupport(extensions.data(), glfwExtensionCount)) {
		throw std::runtime_error("extension requested, but no available!");
	}

	return extensions;
}

void VulkanBase::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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

bool VulkanBase::checkExtensionsSupport(const char** extensionNames, const uint32_t extensionCount)
{
	for (uint32_t i = 0; i < extensionCount; ++i)
	{
		if (!checkExtensionSupport(extensionNames[i]))
			return false;
	}
	return true;
}

bool VulkanBase::checkExtensionSupport(const char* extensionName)
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

int VulkanBase::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	int score = 0;

	// Discrete GPUs have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	// Application can't function without geometry shaders
	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	return score;
}

bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices VulkanBase::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface_, &presentSupport);
		if (presentSupport)
			indices.presentFamily = i;

		if (indices.isComplete()) {
			break;
		}

		++i;
	}

	// Assign index to queue families that could be found
	return indices;
}

