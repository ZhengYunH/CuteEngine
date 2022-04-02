#include "Core/ClientScene.h"
#include "IEntity.h"
#include "Graphics/Common/IRenderScene.h"
#include "IPrimitivesComponent.h"


namespace zyh
{

	void ClientScene::Initialize()
	{
		LoadScene();
		mRenderScene_ = new IRenderScene();
	}

	void ClientScene::Tick()
	{
		DispatchTickEvent();
		CollectPrimitives();
	}

	void ClientScene::AddEntity(IEntity* entity)
	{
		HYBRID_CHECK(std::find(mEntitys_.begin(), mEntitys_.end(), entity) == mEntitys_.end());
		mEntitys_.push_back(entity);
	}

	void ClientScene::DelEntity(IEntity* entity)
	{
		auto itr = std::find(mEntitys_.begin(), mEntitys_.end(), entity);
		if (itr != mEntitys_.end())
			mEntitys_.erase(itr);
	}

	void ClientScene::AddPrimitive(IPrimitivesComponent* prim)
	{
		HYBRID_CHECK(std::find(mPrimitives_.begin(), mPrimitives_.end(), prim) == mPrimitives_.end());
		mPrimitives_.push_back(prim);
	}

	void ClientScene::DelPrimitive(IPrimitivesComponent* prim)
	{
		auto itr = std::find(mPrimitives_.begin(), mPrimitives_.end(), prim);
		if (itr != mPrimitives_.end())
			mPrimitives_.erase(itr);
	}

	bool ClientScene::AddRenderElement(RenderSet renderSet, IRenderElement* element)
	{
		return mRenderScene_->AddRenderElement(renderSet, element);
	}

	const std::vector<IRenderElement*>& ClientScene::GetRenderElements(RenderSet renderSet)
	{
		return mRenderScene_->GetRenderElements(renderSet);
	}

	void ClientScene::LoadScene()
	{
		IEntity* entity = new IEntity();
		entity->AddComponent<IPrimitivesComponent>("Resource/models/viking_room.obj");
		IPrimitivesComponent* comp = entity->GetComponent<IPrimitivesComponent>();
		AddEntity(entity);
	}

	void ClientScene::DispatchTickEvent()
	{
		for (IEntity* entity : mEntitys_)
		{
			entity->Tick();
		}
	}
	void ClientScene::CollectPrimitives()
	{
		mPrimitivesAfterCulling_.clear();

		Culling();
		for (IPrimitivesComponent* prim : mPrimitivesAfterCulling_)
		{
			prim->CollectPrimitives(mRenderScene_);
		}
	}

	void ClientScene::Culling()
	{
		for (IPrimitivesComponent* prim : mPrimitives_)
		{
			if (prim->Culling())
				mPrimitivesAfterCulling_.push_back(prim);
		}
	}

	

}