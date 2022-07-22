#pragma once
#include "Common/Config.h"

namespace zyh
{
	class Archive;

	class IObject
	{
		virtual void Serialize(Archive* Ar) {}
	};
}