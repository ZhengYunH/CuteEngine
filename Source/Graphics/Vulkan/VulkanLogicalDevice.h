#pragma once
#include "VulkanObject.h"
#include "VulkanPhysicalDevice.h"


namespace zyh
{
	class VulkanInstance;
	class VulkanPhysicalDevice;
	class VulkanShader;

	enum E_QUEUE_FAMILY : uint8_t;

	class VulkanLogicalDevice : public TVulkanObject<VkDevice>
	{
	public:
		void connect(VulkanInstance* instance, VulkanPhysicalDevice* physicalDevice);
		void setup() override;
		void cleanup() override;

	private:
		VulkanInstance* mVulkanInstance_{ nullptr };
		VulkanPhysicalDevice* mVulkanPhysicalDevice_{ nullptr };

	public:
		VkQueue getQueue(E_QUEUE_FAMILY family);
		VkQueue graphicsQueue();
		VkQueue presentQueue();

		VulkanShader* getShader(const std::string& shaderFilePath);
		
	private:
		VkQueue				mVkGraphicsQueue_{ VK_NULL_HANDLE };
		VkQueue				mVkPresentQueue_{ VK_NULL_HANDLE };
		std::unordered_map<std::string, VulkanShader*> mShaderCache_;

	public:
		QueueFamilyIndices	mFamilyIndices_;
	};
}