#include "IPrimitivesComponent.h"
#include "File/FileSystem.h"

namespace zyh
{
	void IPrimitivesComponent::Serialize(Archive* ar)
	{
		Super::Serialize(ar);
		ar->AddItem("IPrimitiveTestAttr", "IPrimitiveTestValue");
	}
}

