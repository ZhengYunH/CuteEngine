#include "Common/Config.h"
#include "VulkanObject.h"


namespace zyh
{
	class VulkanMaterial;
	class VulkanLogicalDevice;

	class VulkanDescriptorLayout : public TVulkanObject<VkDescriptorSetLayout>
	{
	public:
		VulkanDescriptorLayout(VulkanMaterial *owner)
			:mOwner_(owner)
		{}

		virtual void connect(VulkanLogicalDevice* logicalDevice);
		virtual void setup() override;
		virtual void cleanup() override;

	protected:
		VulkanLogicalDevice* mVulkanLogicalDevice;

	protected:
		VulkanMaterial* mOwner_;
	};
}