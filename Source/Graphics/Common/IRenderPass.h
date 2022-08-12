#pragma once
// different from VulkanRenderPass
// IRenderPass: Define logical RenderPass, include:
//	- FrameBuffer
//	- Target RenderSet
//	- VulkanRenderBuffer
//	- ...
//	refer vkCmdBeginRenderPass / vkCmdEndRenderPass to understand this class
// VulkanRenderPass: Define Attachment(only metadata, 
//	any pipeline satisfies specific struct can use it) 

#include "Common/Config.h"
#include "Common/KeyCodes.h"

#include "Graphics/Vulkan/VulkanHeader.h"
#include "Graphics/Vulkan/VulkanRenderPass.h"
#include "Graphics/Vulkan/VulkanCommandPool.h"

#include "Core/IPrimitivesComponent.h"
#include "Graphics/Common/RenderResource.h"

#include "Graphics/Imgui/imgui.h"
#include "Graphics/Imgui/imgui_impl_vulkan.h"
#include "Graphics/Imgui/imgui_impl_win32.h"


namespace zyh
{
	class IRenderPass
	{
	public:
		IRenderPass(
			const std::string& renderPassName, 
			const TRenderSets& renderSets,
		):	mName_(renderPassName),
			mRenderSets_(renderSets)
		{
		}

		const TRenderSets& GetRenderSets()
		{
			return mRenderSets_;
		}
	
		const std::vector<RenderTarget*>& GetRenderTargets() { return mRenderTargets_; }
		void AddRenderTarget(RenderTarget* target) { mRenderTargets_.push_back(target); }
		
		const RenderTarget* GetDepthStencilTarget() { return mDepthStencilTarget_; };
		void SetDepthStencilTarget_(RenderTarget* target) { mDepthStencilTarget_ = target; }
	
		virtual void PrepareData() {}

	protected:
		std::string mName_;
		TRenderSets mRenderSets_;
		
		std::vector<RenderTarget*> mRenderTargets_;
		RenderTarget* mDepthStencilTarget_;


	private:
		VkFramebuffer mVKFramebuffer_;
		VkCommandBufferBeginInfo mVKBufferBeginInfo_{};
	};

	struct UISettings {
		bool displayLogos = true;
		bool displayBackground = true;
		bool animateLight = false;
		float lightSpeed = 0.25f;
		std::array<float, 50> frameTimes{};
		float frameTimeMax = 9999.0f, frameTimeMin = 0.0f;
		float lightTimer = 0.0f;
	};

	class ImGuiRenderPass : public IRenderPass
	{
	private:
		UISettings uiSettings;

	public:
		ImGuiRenderPass(
			const std::string& renderPassName,
			const TRenderSets& renderSets,
			VulkanRenderPassBase* renderPass
		);

		virtual void PrepareData() override;

		~ImGuiRenderPass();

		void NewFrame();
		void UpdateBuffers();
		void EmitRenderElements();

	public:
		class VulkanMaterial* mMaterial_{ nullptr };
		class VulkanBuffer* mVertexBuffer_{ nullptr };
		class VulkanBuffer* mIndexBuffer_{ nullptr };
		bool mIsResourceDirty_{ true };

	protected:
		virtual void _DrawElements(VkCommandBuffer vkCommandBuffer) override;

	private:
		void Init();

	protected:
		class ImGuiRenderElement* mRenderElement_;
	};

	class PostProcessRenderPass : public IRenderPass
	{
		struct QuadVert
		{
			glm::vec3 pos;
			glm::vec2 uv;

			static void GetBindingDescriptions(std::vector<VkVertexInputBindingDescription>& descriptions) {
				descriptions = {
					initInputBindingDesc(0, sizeof(QuadVert), VK_VERTEX_INPUT_RATE_VERTEX)
				};
			}

			static void GetAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& descriptions)
			{
				descriptions = {
					initInputAttrDesc(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(QuadVert, pos)), // pos
					initInputAttrDesc(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(QuadVert, uv))	// uv
				};
			}
		};

		class QuadPrimitives : public TPrimitive<QuadVert>
		{
			using Super = TPrimitive<QuadVert>;
		public:
			QuadPrimitives(IMaterial* material)
				: Super(material)
			{
				AddVertex({
					QuadVert{glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec2(0.f, 0.f)},
					QuadVert{glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec2(1.f, 0.f)},
					QuadVert{glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec2(0.f, 1.f)},
					QuadVert{glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec2(1.f, 1.f)}
				});

				AddIndex({
					0, 1, 2,
					1, 3, 2
				});
			}
		};

	public:
		PostProcessRenderPass(
			const std::string& vertShaderFile, 
			const std::string& fragShaderFile,
			const std::string& renderPassName,
			VulkanRenderPassBase* renderPass
		) : IRenderPass(renderPassName, {RenderSet::NONE}, renderPass) // Don't care about Render-Set, cause we handle element
		{
			mMaterial = new IMaterial(vertShaderFile, fragShaderFile);
			mMaterial->GetPipelineState().Rasterization.CullMode = ERasterizationCullMode::NONE;

			mPrim_ = new QuadPrimitives(mMaterial);
		}

		virtual void PrepareData() override
		{
			IRenderPass::PrepareData();
			if (!mElement_)
			{
				mElement_ = new VulkanRenderElement(mPrim_->GetMaterial(), RenderSet::SCENE);
			}
		}

		virtual ~PostProcessRenderPass()
		{
			SafeDestroy(mElement_);
			SafeDestroy(mPrim_);
			SafeDestroy(mMaterial);
		}

		virtual void _DrawElements(VkCommandBuffer vkCommandBuffer)
		{
			mElement_->draw(vkCommandBuffer, GVulkanInstance->GetCurrentImage());
		}

	protected:
		IMaterial* mMaterial{ nullptr };
		QuadPrimitives* mPrim_{ nullptr };
		VulkanRenderElement* mElement_{ nullptr };
	};


	class XRayStencilWritePass : public IRenderPass
	{
		XRayStencilWritePass(
			const std::string& renderPassName,
			const TRenderSets& renderSets,
			VulkanRenderPassBase* renderPass
		): IRenderPass(renderPassName, renderSets, renderPass)
		{
		}
	};

	class XRayPass : public PostProcessRenderPass
	{
	public:
		XRayPass(
			const std::string& renderPassName,
			VulkanRenderPassBase* renderPass
		) : PostProcessRenderPass(
			"Resource/shaders/postprocess.vert.spv",
			"Resource/shaders/postprocess.frag.spv",
			renderPassName,
			renderPass
		)
		{
			mMaterial->GetPipelineState().DepthStencil.DepthTestEnable = false;
			mMaterial->GetPipelineState().DepthStencil.StencilTestEnable = true;
			mMaterial->GetPipelineState().DepthStencil.StencilState.Reference = 1;

			mMaterial->GetPipelineState().ColorBlend.SrcColorBlendFactor = ColorBlendState::EBlendFactor::ONE;
			mMaterial->GetPipelineState().ColorBlend.DstColorBlendFactor = ColorBlendState::EBlendFactor::ZERO;
		}
	};
}