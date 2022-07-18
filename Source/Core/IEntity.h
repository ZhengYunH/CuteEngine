#pragma once
#include "Common/Config.h"
#include "Math/Matrix4x3.h"

namespace zyh
{
	class IComponent;

	class IEntity
	{
	public:
		void Tick();
		void SetTransform(Matrix4x3& mat);
		
		template<typename T, typename... ArgsType>
		IComponent* AddComponent(ArgsType&&... args) 
		{ 
			IComponent* comp = new T(this, std::forward<ArgsType>(args)...);
			mComponents_.push_back(comp);
			return comp;
		}

		template<typename T>
		T* GetComponent() { 
			return static_cast<T*>(mComponents_[0]); 
		}

		const std::vector<IComponent*>& GetComponents() { return mComponents_; }

		void AddUpdateTransformList(IComponent* comp)
		{
			mUpdateTransformList_.push_back(comp);
		}

	protected:
		std::vector<IComponent*> mComponents_;
		std::vector<IComponent*> mUpdateTransformList_;
	};
}