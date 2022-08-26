#include "Renderer.h"
#include "IRenderPass.h"
#include "IRenderScene.h"

#include "Graphics/Vulkan/VulkanBase.h"
#include "Graphics/Vulkan/VulkanRenderElement.h"
#include "Graphics/Vulkan/VulkanRenderPass.h"

#include "Graphics/Vulkan/VulkanLogicalDevice.h"
#include "Graphics/Vulkan/VulkanSurface.h"
#include "Graphics/Vulkan/VulkanSwapchain.h"
#include "Graphics/Common/IRenderPass.h"


namespace zyh
{
	Renderer::Renderer(IRenderScene* renderScene)
		: mRenderScene_(renderScene)
	{
		mPlatform_ = new VulkanBase();
	}

	Renderer::~Renderer()
	{
		
	}

	void Renderer::Build()
	{
		mPlatform_->Initialize();
		RenderTarget target(
			mPlatform_->GetScreenWidth(),
			mPlatform_->GetScreenHeigth(),
			1,
			ETextureType::Texture2D,
			EPixelFormat::A32R32G32B32F
		);
		RenderTarget depthStencil(
			mPlatform_->GetScreenWidth(),
			mPlatform_->GetScreenHeigth(),
			1,
			ETextureType::Texture2D,
			EPixelFormat::D32_SFLOAT_S8_UINT
		);
		target.Quality = ESamplerQuality::Quality8X;
		depthStencil.Quality = ESamplerQuality::Quality8X;


		target.LoadOp = RenderTarget::ELoadOp::DONT_CARE;
		target.StoreOp = RenderTarget::EStoreOp::STORE;
		depthStencil.LoadOp = RenderTarget::ELoadOp::DONT_CARE;
		depthStencil.StoreOp = RenderTarget::EStoreOp::STORE;
		mRenderPasses_.push_back(
			new IRenderPass(
				"Scene",
				{ RenderSet::SCENE }
			)
		);
		mRenderPasses_.back()->AddRenderTarget(target);
		mRenderPasses_.back()->SetDepthStencilTarget(depthStencil);


		target.LoadOp = RenderTarget::ELoadOp::LOAD;
		depthStencil.LoadOp = RenderTarget::ELoadOp::LOAD;
		mRenderPasses_.push_back(
			new IRenderPass(
				"XRayWriter",
				{ RenderSet::XRAY }
			)
		);
		mRenderPasses_.back()->AddRenderTarget(target);
		mRenderPasses_.back()->SetDepthStencilTarget(depthStencil);

		mRenderPasses_.push_back(
			new XRayPass(
				"XRayPostProcess"
			)
		);
		mRenderPasses_.back()->AddRenderTarget(target);
		mRenderPasses_.back()->SetDepthStencilTarget(depthStencil);

		mRenderPasses_.push_back(
			new ImGuiRenderPass(
				"GUI",
				{ RenderSet::SCENE }
		));
		mRenderPasses_.back()->AddRenderTarget(target);
		mRenderPasses_.back()->SetDepthStencilTarget(depthStencil);
	}

	void Renderer::Draw()
	{
		Compile();
		mPlatform_->DrawFrameBegin(mCurrentImage_);
		{
			for (auto renderSet : mRenderScene_->GetExistRenderSets())
			{
				for (auto& renderElement : mRenderScene_->GetRenderElements(renderSet))
				{
					VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
					element->updateUniformBuffer(mCurrentImage_);
				}
			}

			VulkanFrameBuffer* frameBuffer = new VulkanFrameBuffer();
			VulkanRenderPass& renderPass = *mVulkanRenderPasses_[0];
			frameBuffer->createResource(renderPass);
			frameBuffer->setup(renderPass);

			for (VulkanRenderPass* pass : mVulkanRenderPasses_)
			{
				pass->Prepare(frameBuffer->Get());
				pass->Draw();
			}
		}
		mPlatform_->DrawFrameEnd();
	}

	void Renderer::Connect()
	{

	}

	void Renderer::Compile()
	{
		for (auto* renderPass : mVulkanRenderPasses_)
			SafeDestroy(renderPass);

		mVulkanRenderPasses_.clear();
		for (IRenderPass* pass : mRenderPasses_)
		{
			pass->PrepareData();
			VulkanRenderPass* vulkanPass = new VulkanRenderPass(pass);
			vulkanPass->InitailizeResource();
			
			mVulkanRenderPasses_.push_back(vulkanPass);
		}
	}

}