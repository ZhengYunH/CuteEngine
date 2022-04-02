#pragma once
#include "Common/Config.h"
#include "IComponent.h"
#include "Engine.h"
#include "ClientScene.h"
#include "Graphics/Common/IPrimitive.h"
#include "Graphics/Common/ResourceLoader.h"


namespace zyh
{
	class IRenderScene;

	class IPrimitivesComponent : public IComponent
	{
	public:
		IPrimitivesComponent() {}
		IPrimitivesComponent(const std::string& meshFileName)
		{
			GEngine->Scene->AddPrimitive(this);
			LoadData(meshFileName);
		}
		virtual ~IPrimitivesComponent()
		{
			GEngine->Scene->DelPrimitive(this);
		}

	public:
		virtual void LoadData(const std::string& meshFileName)
		{
			IPrimitive* prim = AddPrimitive();
			ResourceLoader::loadModel(meshFileName, prim->mVertices_, prim->mIndices_);
		}
		virtual void CollectPrimitives(IRenderScene* renderScene)
		{
			for (auto& primitive : mPrimitives)
			{
				primitive->CollectPrimitives(renderScene);
			}
		}
		virtual bool Culling() { return true; }

		template<typename... ArgsType>
		IPrimitive* AddPrimitive(ArgsType&&... args)
		{
			IPrimitive* prim = new prim(std::forward<ArgsType>(args)...);
		}

		void AddPrimitive(IPrimitive* primitive)
		{
			mPrimitives.push_back(primitive);
		}

	protected:
		std::vector<IPrimitive*> mPrimitives;
	};
}