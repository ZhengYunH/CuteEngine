#pragma once
#include "Common/Config.h"
#include "Math/Matrix4x3.h"

namespace zyh
{
	class IEntity;

	class IComponent
	{
	public:
		IComponent(IEntity* Parent) :mParent_(Parent) {}
		virtual void Tick() {}
		virtual bool IsTickable() { return mTickable_; }

		IEntity* GetParent() { return mParent_; }
		virtual void UpdateTransform(Matrix4x3& mat) {}
	protected:
		bool mTickable_{ true };
		IEntity* mParent_;
	};
}
