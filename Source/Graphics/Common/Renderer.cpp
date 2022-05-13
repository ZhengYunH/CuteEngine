#include "Renderer.h"
#include "IRenderPass.h"
#include "IRenderScene.h"

#include "Graphics/Vulkan/VulkanBase.h"
#include "Graphics/Vulkan/VulkanRenderElement.h"


#include "Graphics/Vulkan/VulkanLogicalDevice.h"
#include "Graphics/Vulkan/VulkanSurface.h"
#include "Graphics/Vulkan/VulkanSwapchain.h"


namespace zyh
{
	Renderer::Renderer(IRenderScene* renderScene)
		: mRenderScene_(renderScene)
	{
		mPlatform_ = new VulkanBase();
	}

	Renderer::~Renderer()
	{
		ImGui_ImplVulkanH_DestroyWindow(
			GVulkanInstance->mInstance_->Get(), 
			GVulkanInstance->mLogicalDevice_->Get(),
			&mGuiData_,
			nullptr /* Allocator */
		);
	}

	void Renderer::Build()
	{
		mPlatform_->Initialize();

		// GUI
		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		mGuiData_.Surface = GVulkanInstance->mSurface_->Get();
		mGuiData_.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
			GVulkanInstance->mPhysicalDevice_->Get(),
			mGuiData_.Surface,
			requestSurfaceImageFormat,
			(size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
			requestSurfaceColorSpace
		);

#ifdef IMGUI_UNLIMITED_FRAME_RATE
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
		mGuiData_.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
			GVulkanInstance->mPhysicalDevice_->Get(),
			mGuiData_.Surface,
			&present_modes[0],
			IM_ARRAYSIZE(present_modes)
		);

		mRenderPasses_.push_back(
			new IRenderPass(
				"Test",
				{ RenderSet::SCENE },
				GVulkanInstance->mRenderPass_
			)
		);

		mRenderPasses_.push_back(
			new ImGuiRenderPass(
				"GUI",
				{ RenderSet::SCENE },
				GVulkanInstance->mRenderPass_
			)
		);
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = GVulkanInstance->mSwapchain_->getExtend().width;
		io.DisplaySize.y = GVulkanInstance->mSwapchain_->getExtend().height;

		int width, height;
		unsigned char* pixels = NULL;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	}

	void Renderer::Draw()
	{
		// GUI Test
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		bool show_another_window = true;
		
		ImGui::NewFrame();
		// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_another_window);      // Edit bool storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		ImGui::Render();
		static_cast<ImGuiRenderPass*>(mRenderPasses_[1])->mDrawData_ = ImGui::GetDrawData();


		RenderSet renderSet = RenderSet::SCENE;

		mPlatform_->DrawFrameBegin(mCurrentImage_);

		for (auto& renderElement : mRenderScene_->GetRenderElements(renderSet))
		{
			VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
			element->mMaterial_->updateUniformBuffer(mCurrentImage_);
		}

		for (IRenderPass* pass : mRenderPasses_)
		{
			pass->Prepare(GVulkanInstance->GetSwapchainFrameBuffer());
			pass->Draw(renderSet);
		}

		mPlatform_->DrawFrameEnd();
	}

}