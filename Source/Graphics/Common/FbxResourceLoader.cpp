#include "FbxResourceLoader.h"

#include <fbxsdk.h>
#include <windows.h>

namespace zyh
{
	namespace FbxResourceLoader
	{
		static bool isInit = false;

		void InitModule()
		{
			if (isInit)
				return;
			HINSTANCE fbxsdk = LoadLibraryW(L"libfbxsdk.dll");

			isInit = true;
		}

		void _collectMesh(FbxNode* pNode);

		void loadModel(const std::string& modelPath, std::vector<Vertex>& outVertexs, std::vector<uint32_t>& outIndices)
		{
			InitModule();

			FbxManager* lSdkManager = FbxManager::Create();

			// Create the IO settings object.
			FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
			lSdkManager->SetIOSettings(ios);

			// Create an importer using the SDK manager.
			FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

			if (!lImporter->Initialize(modelPath.c_str(), -1, lSdkManager->GetIOSettings()))
			{
				throw std::runtime_error(lImporter->GetStatus().GetErrorString());
			}
			// Create a new scene so that it can be populated by the imported file.
			FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

			// Import the contents of the file into the scene.
			lImporter->Import(lScene);

			// The file is imported, so get rid of the importer.
			lImporter->Destroy();

			FbxNode* lRootNode = lScene->GetRootNode();
			if (lRootNode) {
				auto count = lRootNode->GetChildCount();
				for (int i = 0; i < count; i++)
				{
					FbxNode* pNode = lRootNode->GetChild(i);
					FbxNodeAttribute::EType attr = pNode->GetNodeAttribute()->GetAttributeType();
					if (attr == FbxNodeAttribute::eMesh)
					{
						_collectMesh(pNode);
					}
				}
			}

			lSdkManager->Destroy();
		}

		void _collectMesh(FbxNode* pNode)
		{
			const char* nodeName = pNode->GetName();
			FbxDouble3 translation = pNode->LclTranslation.Get();
			FbxDouble3 rotation = pNode->LclRotation.Get();
			FbxDouble3 scaling = pNode->LclScaling.Get();
			for (int i = 0; i < pNode->GetChildCount(); i++)
			{
				FbxNode* pSubNode = pNode->GetChild(i);
				_collectMesh(pSubNode);
			}
		}

	}
}