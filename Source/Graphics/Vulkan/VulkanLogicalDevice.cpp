#include "VulkanInstance.h"
#include "VulkanLogicalDevice.h"

#include "Graphics/Common/ResourceLoader.h"
#include "VulkanShader.h"


namespace zyh
{
	void VulkanLogicalDevice::connect(VulkanInstance* instance, VulkanPhysicalDevice* physicalDevice)
	{
		mVulkanInstance_ = instance;
		mVulkanPhysicalDevice_ = physicalDevice;
	}

	void VulkanLogicalDevice::setup()
	{
		HYBRID_CHECK(mVulkanInstance_);
		HYBRID_CHECK(mVulkanPhysicalDevice_);

		mFamilyIndices_ = mVulkanPhysicalDevice_->findQueueFamilies();
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { mFamilyIndices_->graphicsFamily.value(), mFamilyIndices_->presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = mFamilyIndices_->graphicsFamily.value();
			queueCreateInfo.queueCount = 1;
			float queuePriority = 1.0f;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &mVulkanPhysicalDevice_->getDeviceFeatures();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(mVulkanPhysicalDevice_->getDeviceExtensions().size());
		createInfo.ppEnabledExtensionNames = mVulkanPhysicalDevice_->getDeviceExtensions().data();
		
		auto validationLayer = mVulkanInstance_->getValidationLayers();
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayer.size());
		createInfo.ppEnabledLayerNames = validationLayer.data();

		VK_CHECK_RESULT(vkCreateDevice(mVulkanPhysicalDevice_->Get(), &createInfo, nullptr, &mVkImpl_), "failed to create logical device!");
	}

	void VulkanLogicalDevice::cleanup()
	{
		vkDestroyDevice(mVkImpl_, nullptr);
	}

	VkQueue VulkanLogicalDevice::getQueue(E_QUEUE_FAMILY family)
	{
		VkQueue queue;
		HYBRID_CHECK(mFamilyIndices_->isQueueFamilyValid(family));
		vkGetDeviceQueue(mVkImpl_, mFamilyIndices_->getIndexByQueueFamily(family), 0, &queue);
		return queue;
	}

	VkQueue VulkanLogicalDevice::graphicsQueue()
	{
		return getQueue(E_QUEUE_FAMILY::GRAPHICS);
	}

	VkQueue VulkanLogicalDevice::presentQueue()
	{
		return getQueue(E_QUEUE_FAMILY::PRESENT);
	}

	VulkanShader* VulkanLogicalDevice::getShader(const std::string& shaderFilePath)
	{
		if (mShaderCache_.find(shaderFilePath) == mShaderCache_.end())
		{
			mShaderCache_.insert({ shaderFilePath, new VulkanShader(shaderFilePath) });
		}
		return mShaderCache_.at(shaderFilePath);
	}
}