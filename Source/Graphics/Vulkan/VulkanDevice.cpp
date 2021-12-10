#include "VulkanDevice.h"

#ifdef _WIN32
LPCWSTR stringToLPCWSTR(const std::string& orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

	return wcstring;
}

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
	if(VulkanBase::S_Instance)
		VulkanBase::S_Instance->handleMessage(hWnd, uMsg, wParam, lParam);
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
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
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

