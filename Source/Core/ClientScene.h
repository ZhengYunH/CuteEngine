#pragma once
#include "Common/Config.h"


namespace zyh
{
	class IEntity;
	
	class IPrimitivesComponent;

	class IRenderElement;
	class IRenderScene;

	class ClientScene
	{
	public:
		void Initialize();
		void Tick();
		void CleanUp() { SafeDestroy(mRenderScene_); }

		void AddEntity(IEntity* entity);
		void DelEntity(IEntity* entity);

		void AddPrimitive(IPrimitivesComponent* prim);

		void DelPrimitive(IPrimitivesComponent* prim);

		/// RenderScene Utility
		bool AddRenderElement(RenderSet renderset, IRenderElement* element);
		const std::vector<IRenderElement*>& GetRenderElements(RenderSet renderSet);

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

		IRenderScene* mRenderScene_;
	};

}