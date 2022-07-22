#include "IComponent.h"
#include "File/FileSystem.h"

namespace zyh
{

	void IComponent::Serialize(Archive* ar)
	{
		ar->AddItem("Tickable", mTickable_);
	}
}