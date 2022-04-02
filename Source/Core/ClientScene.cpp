#include "Core/ClientScene.h"
#include "IEntity.h"
#include "IPrimitivesComponent.h"

namespace zyh
{
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

	void ClientScene::AddRenderElement(RenderSet renderSet, IRenderElement* element)
	{
		if (mRenderElements_.find(renderSet) == mRenderElements_.end())
			mRenderElements_[renderSet] = std::vector<IRenderElement*>();
		mRenderElements_[renderSet].push_back(element);
	}

	const std::vector<IRenderElement*> ClientScene::GetRenderElements(RenderSet renderSet)
	{
		if (mRenderElements_.find(renderSet) != mRenderElements_.end())
			return mRenderElements_[renderSet];
		return std::vector<IRenderElement*>();
	}

	void ClientScene::LoadScene()
	{
		IEntity* entity = new IEntity();
		entity->AddComponent<IPrimitivesComponent>();
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
		mRenderElements_.clear();

		Culling();
		for (IPrimitivesComponent* prim : mPrimitivesAfterCulling_)
		{
			prim->CollectPrimitives();
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