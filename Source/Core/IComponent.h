#pragma once
#include "Common/Config.h"

namespace zyh
{
	class IComponent
	{
	public:
		virtual void Tick() {}
		virtual bool IsTickable() { return mTickable_; }

	protected:
		bool mTickable_{ true };
	};
}
