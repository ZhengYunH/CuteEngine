#pragma once
#include "VulkanObject.h"

namespace zyh
{
	class VulkanInstance;
	class VulkanPhysicalDevice;

	extern enum E_QUEUE_FAMILY : uint8_t;

	class VulkanLogicalDevice : public TVulkanObject<VkDevice>
	{
	public:
		void connect(VulkanInstance* instance, VulkanPhysicalDevice* physicalDevice);
		void setup() override;

	private:
		VulkanInstance* mVulkanInstance_{ nullptr };
		VulkanPhysicalDevice* mVulkanPhysicalDevice_{ nullptr };

	public:
		VkQueue getQueue(E_QUEUE_FAMILY family);
		VkQueue graphicsQueue();
		VkQueue presentQueue();

		VkShaderModule createShaderModule(const std::string& shaderFilePath);
		// TODO may auto destroy, controlled by RTTI
		void destroyShaderModule(VkShaderModule& shaderModule);

	private:
		VkQueue				mVkGraphicsQueue_{ VK_NULL_HANDLE };
		VkQueue				mVkPresentQueue_{ VK_NULL_HANDLE };
		QueueFamilyIndices	mFamilyIndices_;
	};
}