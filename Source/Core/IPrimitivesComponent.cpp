#include "IPrimitivesComponent.h"
#include "File/FileSystem.h"

namespace zyh
{
	void IPrimitivesComponent::Serialize(Archive* ar)
	{
		Super::Serialize(ar);
		ar->AddItem("EPrimitiveType", int(mMeshType_));
		if (!mMeshFileName_.empty())
		{
			ar->AddItem("ResourcePath", mMeshFileName_);
		}
	}
}

