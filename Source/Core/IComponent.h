#pragma once
#include "Common/Config.h"
#include "Math/Matrix4x3.h"
#include "IObject.h"


namespace zyh
{
	class IEntity;

	class IComponent :  public IObject
	{
	public:
		IComponent(IEntity* Parent) :mParent_(Parent) {}
		virtual void Tick() {}
		virtual bool IsTickable() { return mTickable_; }

		IEntity* GetParent() { return mParent_; }
		virtual void UpdateTransform(Matrix4x3& mat) {}

		virtual void Serialize(Archive* ar) override;

	public:
		const std::string& GetName() { return mName_; }

	protected:
		bool mTickable_{ true };
		IEntity* mParent_;
		std::string mName_{ "" };
	};
}
