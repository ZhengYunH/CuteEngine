#pragma once
#include "Common/Config.h"
#include "Core/Engine.h"
#include "Core/IObject.h"
#include "Core/DataStructure/Octree.h"


namespace zyh
{
	class IEntity;
	class Camera;
	
	class IPrimitivesComponent;

	class IRenderElement;
	class IRenderScene;

	class ClientScene : virtual IObject
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

	public:
		virtual void Serialize(class Archive* ar) override;
		IRenderScene* GetRenderScene() { return mRenderScene_; }

	protected:
		void LoadScene();
		void SaveScene();

	protected:
		void DispatchTickEvent();
		void CollectRenderElements(RenderSet renderSet);
		void Culling(RenderSet renderSet);
		void DispatchOSMessage();

	public:
		Camera* GetCamera() { return mCamera_; }

	private:
		std::vector<IEntity*> mEntitys_;
		std::vector<IPrimitivesComponent*> mPrimitives_;
		std::vector<IPrimitivesComponent*> mPrimitivesAfterCulling_;

		IRenderScene* mRenderScene_;
		class Renderer* mRenderer_;
		Camera* mCamera_;

		Octree<IPrimitivesComponent> mPrimitiveTree_;
	};

}