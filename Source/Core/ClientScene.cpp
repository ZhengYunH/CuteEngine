#include "Core/ClientScene.h"
#include "IEntity.h"
#include "Graphics/Common/IRenderScene.h"
#include "IPrimitivesComponent.h"
#include "Camera/Camera.h"
#include "Graphics/Common/Renderer.h"

#include "File/FileSystem.h"


namespace zyh
{
	void ClientScene::Initialize()
	{
		mRenderScene_ = new IRenderScene();
		mRenderer_ = new Renderer(mRenderScene_);
		mCamera_ = new Camera();

		mRenderer_->Build();
		LoadScene();
	}

	void ClientScene::Tick()
	{
		DispatchOSMessage();
		DispatchTickEvent();
		mCamera_->tick(GEngine->GetDeltaTime());

		// Tick Render Scene(TODO: MultiThread)
		mRenderer_->Draw();
	}

	void ClientScene::CleanUp()
	{
		SafeDestroy(mRenderScene_);
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
		SceneXmlParser parser("Resource/files/scene.xml");
		parser.Load();
		for (auto* entity : parser.GetEntities())
		{
			AddEntity(entity);
		}
		CollectAllRenderElements();
	}

	void ClientScene::DispatchTickEvent()
	{
		for (IEntity* entity : mEntitys_)
		{
			entity->Tick();
		}
	}

	void ClientScene::CollectAllRenderElements()
	{
		CollectRenderElements(RenderSet::SCENE);
	}

	void ClientScene::CollectRenderElements(RenderSet renderSet)
	{
		mPrimitivesAfterCulling_.clear();
		mRenderScene_->Clear(renderSet);

		Culling(renderSet);
		for (IPrimitivesComponent* prim : mPrimitivesAfterCulling_)
		{
			prim->EmitRenderElements(renderSet, *mRenderScene_);
		}
	}

	void ClientScene::Culling(RenderSet renderSet)
	{
		for (IPrimitivesComponent* prim : mPrimitives_)
		{
			if (prim->Culling())
				mPrimitivesAfterCulling_.push_back(prim);
		}
	}

	void ClientScene::DispatchOSMessage()
	{
#if defined(_WIN32)
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				return;
			}
		}
#endif
	}

}