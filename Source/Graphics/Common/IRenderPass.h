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
#include "Graphics/Vulkan/VulkanCommandPool.h"

#include "Core/IPrimitivesComponent.h"
#include "Graphics/Common/RenderResource.h"

#include "Graphics/Imgui/imgui.h"
#include "Graphics/Imgui/imgui_impl_vulkan.h"
#include "Graphics/Imgui/imgui_impl_win32.h"


namespace zyh
{
	class VulkanRenderElement;
	
	class IRenderPass
	{
	public:
		IRenderPass(
			const std::string& renderPassName, 
			const TRenderSets& renderSets
		):	mName_(renderPassName),
			mRenderSets_(renderSets)
		{
		}

		const TRenderSets& GetRenderSets()
		{
			return mRenderSets_;
		}
	
		const std::vector<RenderTarget>& GetRenderTargets() { return mRenderTargets_; }
		void AddRenderTarget(RenderTarget& target) 
		{ 
			mRenderTargets_.push_back(target); 
			mWidth_ = Min(mWidth_, target.Width);
			mHeight_ = Min(mHeight_, target.Height);
		}
		
		const RenderTarget& GetDepthStencilTarget() { return mDepthStencilTarget_; };
		void SetDepthStencilTarget(RenderTarget& target) 
		{ 
			mDepthStencilTarget_ = target;
			mWidth_ = Min(mWidth_, target.Width);
			mHeight_ = Min(mHeight_, target.Height);
		}
	
		virtual void PrepareData() {}

		uint32_t GetWidth() { return mWidth_; }
		uint32_t GetHeight() { return mHeight_; }

	protected:
		std::string mName_;
		TRenderSets mRenderSets_;
		
		std::vector<RenderTarget> mRenderTargets_;
		RenderTarget mDepthStencilTarget_;

		uint32_t mWidth_{ MAX_UINT };
		uint32_t mHeight_{ MAX_UINT };

	private:
		VkFramebuffer mVKFramebuffer_;
		VkCommandBufferBeginInfo mVKBufferBeginInfo_{};
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
			const std::string& renderPassName
		) : IRenderPass(renderPassName, { RenderSet::POSTPROCESS })
		{
			mMaterial = new IMaterial(vertShaderFile, fragShaderFile, RenderSet::POSTPROCESS);
			mMaterial->GetPipelineState().Rasterization.CullMode = ERasterizationCullMode::NONE;

			mPrim_ = new QuadPrimitives(mMaterial);
		}

		virtual void PrepareData() override;
		virtual ~PostProcessRenderPass();

	protected:
		IMaterial* mMaterial{ nullptr };
		QuadPrimitives* mPrim_{ nullptr };
		VulkanRenderElement* mElement_{ nullptr };
	};

	class XRayStencilWritePass : public IRenderPass
	{
		XRayStencilWritePass(
			const std::string& renderPassName,
			const TRenderSets& renderSets
		): IRenderPass(renderPassName, renderSets)
		{
		}
	};

	class XRayPass : public PostProcessRenderPass
	{
	public:
		XRayPass(
			const std::string& renderPassName
		) : PostProcessRenderPass(
			"Resource/shaders/postprocess.vert.spv",
			"Resource/shaders/postprocess.frag.spv",
			renderPassName
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