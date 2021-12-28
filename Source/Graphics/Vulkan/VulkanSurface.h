#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanInstance;

	class VulkanSurface : public TVulkanObject<VkSurfaceKHR>
	{
	public:
		void connect(VulkanInstance* instance);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
		void setup(void* platformHandle, void* platformWindow);
#endif
		void cleanup() override;

	private:
		VulkanInstance* mVulkanInstance_;
	};
}