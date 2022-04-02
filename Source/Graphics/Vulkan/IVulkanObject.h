#pragma once


namespace zyh
{
	/// <summary>
	/// Interface for all Vulkan class, must implement 2 method
	///		setup
	///		cleanup
	/// </summary>
	class IVulkanObject
	{
	public:
		virtual void connect() {};
		virtual void setup() = 0;
		virtual void cleanup() = 0;
	};
}