#pragma once
#include "Common/Config.h"


namespace zyh
{
	struct IRenderStage
	{

	};

	struct RasterizationState
	{
		enum class ERasterizationCullMode
		{
			NONE = 0,
			FRONT,
			BACK,
			FRONT_AND_BACK,
		};

		ERasterizationCullMode CullMode{ ERasterizationCullMode::BACK };
	};
	using ERasterizationCullMode = RasterizationState::ERasterizationCullMode;
	static RasterizationState DefaultRasterizationState{};

	enum class ECompareOP : uint8_t
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_OR_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_OR_EQUAL,
		ALWAYS,
	};

	struct DepthStencilState
	{
		enum class EStencilOp
		{
			KEEP = 0,
			ZERO = 1,
			REPLACE = 2,
			INCREMENT_AND_CLAMP = 3,
			DECREMENT_AND_CLAMP = 4,
			INVERT = 5,
			INCREMENT_AND_WRAP = 6,
			DECREMENT_AND_WRAP = 7,
		};

		// how to influence the stencil buffer
		struct StencialState
		{
			EStencilOp FailOp{ EStencilOp::KEEP };
			EStencilOp PassOp{ EStencilOp::KEEP };
			EStencilOp DepthFailOp{ EStencilOp::KEEP }; // Depth Fail but Stencil Pass
			ECompareOP CompareOp{ ECompareOP::EQUAL };
			uint32_t Reference{ 1 };
		};

		bool DepthTestEnable{ true };
		bool DepthWriteEnable{ true };
		bool StencilTestEnable{ false };
		ECompareOP DepthCompareOp{ ECompareOP::LESS_OR_EQUAL };
		StencialState StencilState{};
	};
	static DepthStencilState DefaultDepthStencilState{};

	// IPipelineState
	struct IPipelineState
	{
		RasterizationState Rasterization{ DefaultRasterizationState };
		DepthStencilState DepthStencil{ DefaultDepthStencilState };
	};

	static IPipelineState DefaultPipelineState;
}