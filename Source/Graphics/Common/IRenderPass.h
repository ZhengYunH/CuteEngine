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
	
	struct RenderTargetWrapper
	{
		RenderTargetWrapper() = default;

		RenderTargetWrapper(RenderTarget& desc, bool isDepthStencil)
			: RenderTargetDesc(desc), IsDepthStencil(isDepthStencil)
		{

		}
		RenderTarget RenderTargetDesc;
		bool IsDepthStencil;
		bool IsSwapChain{ false };
		bool IsResolve{ false };
	};

	class RenderTargetResource : IRenderTarget
	{
	public:
		enum class ResourceState
		{
			EResourceState_None,
			EResourceState_Creating,
			EResourceState_Created,
			EResourceState_Destroyed
		};

		RenderTargetResource(RenderTargetWrapper targetDesc)
		{
			Desc = targetDesc;
		}

		virtual bool Create() 
		{
			if (State == ResourceState::EResourceState_None)
			{
				State = ResourceState::EResourceState_Creating;
				Create_Imp();
				State = ResourceState::EResourceState_Created;
				return true;
			}
			return false;
		};

		virtual void Create_Imp() = 0;

		void AddRef() { RefCount += 1; }
		void DelRef() { RefCount -= 1; }

		RenderTargetWrapper Desc;
		int RefCount = 0;
		ResourceState State{ ResourceState::EResourceState_None };
	};

	class RenderDevice
	{
	public:
		// return Target index
		uint32_t CreateRenderTarget(RenderTarget& targetDesc)
		{
			RenderTargetResource* resource = CreateRenderTarget_Imp(RenderTargetWrapper(targetDesc, false));
			RenderTargets.push_back(resource);
			return static_cast<uint32_t>(RenderTargets.size() - 1);
		}

		uint32_t CreateDepthStencil(RenderTarget& targetDesc)
		{
			RenderTargetResource* resource = CreateRenderTarget_Imp(RenderTargetWrapper(targetDesc, true));
			RenderTargets.push_back(resource);
			return static_cast<uint32_t>(RenderTargets.size() - 1);
		}

		uint32_t CreateSwapChain(RenderTarget& targetDesc)
		{
			RenderTargetWrapper wrapper(targetDesc, false);
			wrapper.IsResolve = true;
			wrapper.IsSwapChain = true;
			RenderTargetResource* resource = CreateRenderTarget_Imp(wrapper);
			RenderTargets.push_back(resource);
			return static_cast<uint32_t>(RenderTargets.size() - 1);
		}

		RenderTargetResource& GetRenderTargetResource(uint32_t index)
		{
			HYBRID_CHECK(index < RenderTargets.size());
			return *RenderTargets[index];
		}

		virtual RenderTargetResource* CreateRenderTarget_Imp(RenderTargetWrapper targetDesc) = 0;
		std::vector<RenderTargetResource*> RenderTargets;
	};
	extern RenderDevice* GRenderDevice;

	class IRenderNode
	{
	public:
		IRenderNode() = default;
		~IRenderNode()
		{
			for (uint32_t targetIdx : ReadTargets)
			{
				RenderTargetResource& target = GRenderDevice->GetRenderTargetResource(targetIdx);
				target.DelRef();
			}

			for (uint32_t targetIdx : WriteTargets)
			{
				RenderTargetResource& target = GRenderDevice->GetRenderTargetResource(targetIdx);
				target.DelRef();
			}
		}

		uint32_t CreateRenderTarget(RenderTarget& target, bool needResolve = false)
		{
			uint32_t renderTargetIdx = GRenderDevice->CreateRenderTarget(target);
			uint32_t resolveIdx = 0;
			if (needResolve)
			{
				resolveIdx = GRenderDevice->CreateRenderTarget(target);
			}
			Write(renderTargetIdx, resolveIdx);
			return renderTargetIdx;
		}

		uint32_t CreateDepthStencil(RenderTarget& target)
		{
			uint32_t depthStencilIdx = GRenderDevice->CreateDepthStencil(target);
			SetDepthStencil(depthStencilIdx);
			return depthStencilIdx;
		}

		uint32_t CreateSwapChain(RenderTarget& target)
		{
			uint32_t swapChain = GRenderDevice->CreateSwapChain(target);
			Write(swapChain);
			return swapChain;
		}

		virtual void CreateRenderResource()
		{
			for (uint32_t idx : WriteTargets)
			{
				bool isInit = GRenderDevice->GetRenderTargetResource(idx).Create();
				if (isInit)
					CreateTargets.push_back(idx);
			}

			bool isInit = GRenderDevice->GetRenderTargetResource(DepthStencil).Create();
			if (isInit)
				CreateTargets.push_back(DepthStencil);

			// Check Resource Validation
			for (uint32_t idx : ReadTargets)
			{
				RenderTargetResource& target = GRenderDevice->GetRenderTargetResource(idx);
				HYBRID_CHECK(target.State == RenderTargetResource::ResourceState::EResourceState_Created);
			}
		}

		void Write(uint32_t targetIdx, uint32_t resolveIdx = 0) {
			RenderTargetResource& target = GRenderDevice->GetRenderTargetResource(targetIdx);
			target.AddRef();
			if (resolveIdx)
			{
				ResolveMap.insert({ resolveIdx, targetIdx });
			}
			WriteTargets.push_back(targetIdx);

			mHeight_ = Min(mHeight_, target.Desc.RenderTargetDesc.Height);
			mWidth_ = Min(mWidth_, target.Desc.RenderTargetDesc.Width);
		}
		
		void Read(uint32_t targetIdx) {
			RenderTargetResource& target = GRenderDevice->GetRenderTargetResource(targetIdx);
			target.AddRef();
			ReadTargets.push_back(targetIdx);
		}

		void SetDepthStencil(uint32_t depthStencilIdx)
		{
			DepthStencil = depthStencilIdx;
		}


		uint32_t GetWidth() { return mWidth_; }
		uint32_t GetHeight() { return mHeight_; }

	protected:
		std::vector<uint32_t> ReadTargets;
		std::vector<uint32_t> WriteTargets;
		std::unordered_map<uint32_t, uint32_t> ResolveMap;
		std::vector<uint32_t> CreateTargets;
		uint32_t DepthStencil = 0;

		uint32_t mWidth_{ MAX_UINT };
		uint32_t mHeight_{ MAX_UINT };
	};

	class IRenderPass : public IRenderNode
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
	
		virtual std::vector<RenderTarget> GetWriteTargetsDesc() 
		{ 
			std::vector<RenderTarget> renderTargetsDesc;
			for (uint32_t idx : WriteTargets)
			{
				RenderTargetWrapper& wrapper = GRenderDevice->GetRenderTargetResource(idx).Desc;

				if (wrapper.IsSwapChain)
					continue;

				RenderTarget& targetDesc = wrapper.RenderTargetDesc;
				
				if (std::find(CreateTargets.begin(), CreateTargets.end(), idx) != CreateTargets.end())
				{
					targetDesc.LoadOp = RenderTarget::ELoadOp::DONT_CARE;
					targetDesc.StoreOp = RenderTarget::EStoreOp::STORE;
				}
				else
				{
					targetDesc.LoadOp = RenderTarget::ELoadOp::LOAD;
					targetDesc.StoreOp = RenderTarget::EStoreOp::STORE;
				}
				renderTargetsDesc.push_back(targetDesc);
			}
			return renderTargetsDesc;
		}

		virtual std::vector<RenderTargetResource*> GetAllWriteTargets()
		{
			std::vector<RenderTargetResource*> targets;
			for (uint32_t idx : WriteTargets)
			{
				targets.push_back(&GRenderDevice->GetRenderTargetResource(idx));
			}
			if (DepthStencil)
				targets.push_back(&GRenderDevice->GetRenderTargetResource(DepthStencil));
			return targets;
		}

		virtual std::vector<RenderTarget> GetResolveTargetsDesc()
		{
			return std::vector<RenderTarget>();
		}
		
		RenderTarget GetDepthStencilTargetDesc() 
		{ 
			if (DepthStencil == 0)
			{
				return RenderTarget();
			}

			RenderTarget targetDesc = GRenderDevice->GetRenderTargetResource(DepthStencil).Desc.RenderTargetDesc;
			if (std::find(CreateTargets.begin(), CreateTargets.end(), DepthStencil) != CreateTargets.end())
			{
				targetDesc.LoadOp = RenderTarget::ELoadOp::DONT_CARE;
				targetDesc.StoreOp = RenderTarget::EStoreOp::STORE;
			}
			else
			{
				targetDesc.LoadOp = RenderTarget::ELoadOp::LOAD;
				targetDesc.StoreOp = RenderTarget::EStoreOp::STORE;
			}
			return targetDesc;
		};
	
		virtual void PrepareData() {}

	protected:
		std::string mName_;
		TRenderSets mRenderSets_;
		
	private:
		VkFramebuffer mVKFramebuffer_;
		VkCommandBufferBeginInfo mVKBufferBeginInfo_{};
	};

	class FinalPass : public IRenderPass
	{
	public:
		FinalPass(
			const std::string& renderPassName,
			const TRenderSets& renderSets
		) : IRenderPass(renderPassName, renderSets)
		{
		}

		virtual std::vector<RenderTarget> GetResolveTargetsDesc() override
		{
			return std::vector<RenderTarget>({ RenderTarget() });
		}
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