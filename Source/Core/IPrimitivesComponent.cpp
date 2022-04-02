#include "IPrimitivesComponent.h"
#include "Engine.h"
#include "ClientScene.h"

namespace zyh
{
	IPrimitivesComponent::IPrimitivesComponent()
	{
		GEngine->Scene->AddPrimitive(this);
	}

	IPrimitivesComponent::~IPrimitivesComponent()
	{
		GEngine->Scene->DelPrimitive(this);
	}

	void IPrimitivesComponent::Tick()
	{

	}

	void IPrimitivesComponent::CollectPrimitives()
	{

	}
}