#pragma once
#include "Common/Config.h"

namespace zyh
{
	enum class EPixelFormat
	{
		UNDEFINED = 0,
		A32R32G32B32F,
		A16B16G16R16F,
		R8G8B8A8,
		D32_SFLOAT_S8_UINT,
	};

	enum class ETextureType
	{
		None = 0,
		Texture2D,
		Texture3D,
		TextureCube,
	};

	enum class ESamplerQuality
	{
		None = 0,
		Quality2X,
		Quality4X,
		Quality8X,
	};

	struct IRenderResource
	{
	};

	struct ITexture : public IRenderResource
	{
		ITexture(uint16_t width, uint16_t height, uint16_t mips, ETextureType type, EPixelFormat format)
			: Width(width)
			, Height(height)
			, Mips(mips)
			, Type(type)
			, Format(format)
		{
		}

		uint16_t Width;
		uint16_t Height;
		uint16_t Mips;
		ETextureType Type;
		EPixelFormat Format;
		ESamplerQuality Quality{ ESamplerQuality::None };
	};

	struct IRenderTarget : public ITexture
	{
		IRenderTarget(uint16_t width, uint16_t height, uint16_t mips, ETextureType type, EPixelFormat format)
			: ITexture(width, height, mips, type, format)
		{
		}
	};

	struct RenderTarget : public IRenderTarget
	{
		RenderTarget(uint16_t width, uint16_t height, uint16_t mips, ETextureType type, EPixelFormat format)
			: IRenderTarget(width, height, mips, type, format)
		{
		}

		enum class ELoadOp
		{
			LOAD = 0,
			CLEAR = 1,
			DONT_CARE = 2
		};

		enum class EStoreOp 
		{
			STORE,
			DONT_CARE
		};

		ELoadOp LoadOp{ ELoadOp::DONT_CARE };
		EStoreOp StoreOp{ EStoreOp::DONT_CARE };
	};
}