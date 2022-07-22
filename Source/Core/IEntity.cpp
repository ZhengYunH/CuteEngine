#include "IEntity.h"
#include "IComponent.h"
#include "File/FileSystem.h"

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
		mTransform_ = mat;
		for (IComponent* comp : mUpdateTransformList_)
		{
			comp->UpdateTransform(mat);
		}
	}

	void IEntity::Serialize(Archive* ar)
	{
		ar->BeginSection("Components");
		{
			for (IComponent* comp : mComponents_)
			{
				ar->BeginSection(comp->GetName());
				{
					comp->Serialize(ar);
				}
				ar->EndSection();
			}
		}
		ar->EndSection();
		ar->AddItem("Transform", mTransform_);
	}

}