#pragma once
#include "Common/Config.h"


namespace zyh
{
	class IEntity;
	class Camera;
	
	class IPrimitivesComponent;

	class IRenderElement;
	class IRenderScene;

	class ClientScene
	{
	public:
		void Initialize();
		void Tick();
		void CleanUp();

		void AddEntity(IEntity* entity);
		void DelEntity(IEntity* entity);

		void AddPrimitive(IPrimitivesComponent* prim);
		void DelPrimitive(IPrimitivesComponent* prim);

		/// RenderScene Utility
		bool AddRenderElement(RenderSet renderset, IRenderElement* element);
		const std::vector<IRenderElement*>& GetRenderElements(RenderSet renderSet);

		void CollectAllRenderElements();

	protected:
		void LoadScene();

	protected:
		void DispatchTickEvent();
		void CollectRenderElements(RenderSet renderSet);
		void Culling(RenderSet renderSet);

	public:
		Camera* GetCamera() { return mCamera_; }

	private:
		std::vector<IEntity*> mEntitys_;
		std::vector<IPrimitivesComponent*> mPrimitives_;
		std::vector<IPrimitivesComponent*> mPrimitivesAfterCulling_;

		IRenderScene* mRenderScene_;
		Camera* mCamera_;
	};

}