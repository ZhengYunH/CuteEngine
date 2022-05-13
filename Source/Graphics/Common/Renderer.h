#pragma once
#include "Common/Config.h"
#include "Graphics/Imgui/imgui.h"
#include "Graphics/Imgui/imgui_impl_vulkan.h"
#include "Graphics/Imgui/imgui_impl_win32.h"


namespace zyh
{
	typedef ImGui_ImplVulkanH_Window ImGuiType;

	class IRenderPass;
	class IRenderScene;
	class VulkanBase;

	class Renderer
	{
	public:
		Renderer(IRenderScene* renderScene);
		virtual ~Renderer();

		void Build();
		void Draw();

	protected:
		VulkanBase* mPlatform_;
		IRenderScene* mRenderScene_;
		std::vector<IRenderPass*> mRenderPasses_;

		size_t mCurrentImage_ = 0;

	private:
		ImGuiType mGuiData_;
		uint32_t mGuiMinImageCount_ = 2;
	};
}