#pragma once
#include "Common/Config.h"
#include "IComponent.h"
#include "Graphics/Common/IPrimitive.h"


namespace zyh
{
	class IPrimitivesComponent : public IComponent
	{
	public:
		IPrimitivesComponent();
		virtual ~IPrimitivesComponent();

	public:
		virtual void LoadData();

		virtual void Tick() override;
		virtual void CollectPrimitives();
		virtual bool Culling() { return true; }

	protected:
		std::vector<IPrimitive*> mPrimitives;
	};
}