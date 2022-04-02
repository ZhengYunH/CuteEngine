#include "IEntity.h"
#include "IComponent.h"

namespace zyh
{
	void IEntity::Tick()
	{
		for (IComponent* comp : mComponents_)
		{
			if(!comp->IsTickable())
				continue;
			comp->Tick();
		}
	}
}