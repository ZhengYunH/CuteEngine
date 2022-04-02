#pragma once
#include "Common/Config.h"


namespace zyh
{
	class IEntity;
	class IPrimitivesComponent;
	class IRenderElement;

	class ClientScene
	{
	public:
		void Initialize() { LoadScene(); }
		void Tick();
		void CleanUp() {}

		void AddEntity(IEntity* entity);
		void DelEntity(IEntity* entity);

		void AddPrimitive(IPrimitivesComponent* prim);
		void DelPrimitive(IPrimitivesComponent* prim);

		void AddRenderElement(RenderSet renderset, IRenderElement* element);
		const std::vector<IRenderElement*> GetRenderElements(RenderSet renderSet);

	protected:
		void LoadScene();

	protected:
		void DispatchTickEvent();
		void CollectPrimitives();
		void Culling();

	private:
		std::vector<IEntity*> mEntitys_;
		std::vector<IPrimitivesComponent*> mPrimitives_;
		std::vector<IPrimitivesComponent*> mPrimitivesAfterCulling_;

		std::map<RenderSet, std::vector<IRenderElement*>> mRenderElements_;
	};

}