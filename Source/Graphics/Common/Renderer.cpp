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
		SetupPipeline();
		Compile();
	}

	void Renderer::Draw()
	{
		mPlatform_->DrawFrameBegin(mCurrentImage_);
		{
			for (auto renderSet : mRenderScene_->GetExistRenderSets())
			{
				std::vector<IRenderElement*> elements;
				mRenderScene_->GetRenderElements(renderSet, elements);
				for (auto& renderElement : elements)
				{
					VulkanRenderElement* element = static_cast<VulkanRenderElement*>(renderElement);
					element->updateUniformBuffer(mCurrentImage_);
				}
			}


			for (VulkanRenderPass* pass : mVulkanRenderPasses_)
			{
				pass->Prepare();
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

		// Resolve RenderPass
		for (IRenderPass* pass : mRenderPasses_)
		{
			pass->PrepareData();
			VulkanRenderPass* vulkanPass = new VulkanRenderPass(pass);
			vulkanPass->InitailizeResource();
			
			mVulkanRenderPasses_.push_back(vulkanPass);
		}
	}

	void Renderer::SetupPipeline()
	{
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

		IRenderPass* pass;
		mRenderPasses_.push_back(
			new IRenderPass(
				"Detonate",
				{ RenderSet::NONE }
			)
		);
		pass = mRenderPasses_.back();
		uint32_t renderTargetIdx = pass->CreateRenderTarget(target);
		uint32_t depthStencilTargetIdx = pass->CreateDepthStencil(depthStencil);

		mRenderPasses_.push_back(
			new IRenderPass(
				"Scene",
				{ RenderSet::SCENE }
			)
		);
		pass = mRenderPasses_.back();
		pass->Write(renderTargetIdx);
		pass->SetDepthStencil(depthStencilTargetIdx);

		target.LoadOp = RenderTarget::ELoadOp::LOAD;
		depthStencil.LoadOp = RenderTarget::ELoadOp::LOAD;
		mRenderPasses_.push_back(
			new IRenderPass(
				"XRayWriter",
				{ RenderSet::XRAY }
			)
		);
		pass = mRenderPasses_.back();
		pass->Write(renderTargetIdx);
		pass->SetDepthStencil(depthStencilTargetIdx);

		mRenderPasses_.push_back(
			new XRayPass(
				"XRayPostProcess"
			)
		);
		pass = mRenderPasses_.back();
		pass->Write(renderTargetIdx);
		pass->SetDepthStencil(depthStencilTargetIdx);

		mRenderPasses_.push_back(
			new FinalPass(
				"Final",
				{ RenderSet::NONE }
			)
		);
		pass = mRenderPasses_.back();
		pass->Read(renderTargetIdx);
		RenderTarget SwapChainTarget(
			mPlatform_->GetScreenWidth(),
			mPlatform_->GetScreenHeigth(),
			1,
			ETextureType::Texture2D,
			EPixelFormat::R8G8B8A8
		);
		SwapChainTarget.Quality = ESamplerQuality::Quality8X;
		pass->CreateRenderTarget(SwapChainTarget);
		pass->CreateSwapChain(SwapChainTarget);
	}
}