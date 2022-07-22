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

	void IEntity::SetTransform(Matrix4x3& mat)
	{
		for (IComponent* comp : mUpdateTransformList_)
		{
			comp->UpdateTransform(mat);
		}
	}

	void IEntity::Serialize(Archive* ar)
	{
		//ar->BeginSection("Entity");
		//for (IComponent* comp : mComponents_)
		//{
		//	// comp->Serialize(ar);
		//}
		//ar->BeginSection("End");
	}

}