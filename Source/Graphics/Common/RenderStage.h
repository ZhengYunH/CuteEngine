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
		bool DepthTestEnable{ true };
		bool DepthWriteEnable{ true };
		ECompareOP DepthCompareOp{ ECompareOP::LESS_OR_EQUAL };
	};
	static DepthStencilState DefaultDepthStencilState{};

	// IPipelineState
	struct IPipelineState
	{

	};
}