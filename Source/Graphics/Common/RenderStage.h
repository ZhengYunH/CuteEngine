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

	struct ColorBlendState
	{
		enum class EBlendFactor
		{
			ZERO = 0,
			ONE = 1,
			SRC_COLOR = 2,
			ONE_MINUS_SRC_COLOR = 3,
			DST_COLOR = 4,
			ONE_MINUS_DST_COLOR = 5,
			SRC_ALPHA = 6,
			ONE_MINUS_SRC_ALPHA = 7,
			DST_ALPHA = 8,
			ONE_MINUS_DST_ALPHA = 9,
			CONSTANT_COLOR = 10,
			ONE_MINUS_CONSTANT_COLOR = 11,
			CONSTANT_ALPHA = 12,
			ONE_MINUS_CONSTANT_ALPHA = 13
		};

		enum class EBlendOP
		{
			ADD = 0,
			SUBSTRACT = 1,
			REVERSE_SUBSTRACT = 2,
			MIN = 3,
			MAX = 4
		};

		bool BlendEnable{ true };

		EBlendFactor SrcColorBlendFactor{ EBlendFactor::SRC_ALPHA };
		EBlendFactor DstColorBlendFactor{ EBlendFactor::ONE_MINUS_SRC_ALPHA };
		EBlendOP ColorBlendOp{ EBlendOP::ADD };

		EBlendFactor SrcAlphaBlendFactor{ EBlendFactor::ONE_MINUS_SRC_ALPHA };
		EBlendFactor DstAlphaBlendFactor{ EBlendFactor::ZERO };
		EBlendOP AlphaBlendOp{ EBlendOP::ADD };
	};
	static ColorBlendState DefaultColorBlendState{};


	// IPipelineState
	struct IPipelineState
	{
		RasterizationState Rasterization{ DefaultRasterizationState };
		DepthStencilState DepthStencil{ DefaultDepthStencilState };
		ColorBlendState ColorBlend{ DefaultColorBlendState };
	};

	static IPipelineState DefaultPipelineState{};
}