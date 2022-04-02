#pragma once
#include "Common/Config.h"

namespace zyh
{
	class IComponent
	{
	public:
		virtual void Tick() = 0;
		virtual bool IsTickable() { return mTickable_; }

	protected:
		bool mTickable_{ true };
	};
}
