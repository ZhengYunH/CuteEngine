#pragma once
#include "Common/Config.h"

namespace zyh
{
	class IComponent;

	class IEntity
	{
	public:
		void Tick();
		
		template<typename T>
		void AddComponent() { mComponents_.push_back(new T()); }

		template<typename T>
		T* GetComponent() { return static_cast<T*>(mComponents_[0]); }

		const std::vector<IComponent*>& GetComponents() { return mComponents_; }

	protected:
		std::vector<IComponent*> mComponents_;
	};
}