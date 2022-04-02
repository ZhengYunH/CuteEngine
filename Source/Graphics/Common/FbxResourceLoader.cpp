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

			std::unordered_map<Vertex, uint32_t> uniqueVertices{};
			for (int meshIndex = 0; meshIndex < 1/*lScene->GetGeometryCount()*/; ++meshIndex)
			{
				const FbxMesh* pMesh = static_cast<const FbxMesh*>(lScene->GetGeometry(meshIndex));
				if (pMesh)
				{
					const char* nodeName = pMesh->GetName();
					
					auto ttt = pMesh->GetPolygonCount();
					for (int polygonCount = 0; polygonCount < pMesh->GetPolygonCount(); ++polygonCount)
					{
						int polygonTotal = pMesh->GetPolygonSize(polygonCount);
						int polygonStart = pMesh->GetPolygonVertexIndex(polygonCount);

						std::vector<uint32_t> indiceArray(polygonTotal);
						for (int vertIndex = 0; vertIndex < polygonTotal; ++vertIndex)
						{
							Vertex vertex{};
							int vertexIndex = pMesh->GetPolygonVertices()[polygonStart + vertIndex];
							const auto& vertPos = pMesh->GetControlPointAt(vertexIndex);
							vertex.pos = { vertPos[0], vertPos[1], vertPos[2] };
							vertex.color = { 1.0f, 1.0f, 1.0f };
							
							if (uniqueVertices.count(vertex) == 0) {
								uniqueVertices[vertex] = static_cast<uint32_t>(outVertexs.size());
								outVertexs.push_back(vertex);
							}

							indiceArray[vertIndex] = uniqueVertices[vertex];
						}

						for (size_t polygonCount = 0; polygonCount <= polygonTotal - 3; ++polygonCount)
						{
							outIndices.push_back(indiceArray[polygonCount]);
							outIndices.push_back(indiceArray[polygonCount + 1]);
							outIndices.push_back(indiceArray[polygonCount + 2]);


							/*outIndices.push_back(indiceArray[polygonCount]);
							outIndices.push_back(indiceArray[polygonCount + polygonCount % 2 + 1]);
							outIndices.push_back(indiceArray[polygonCount + 2 - polygonCount % 2]);*/
						}
					}
				}
			}

			lSdkManager->Destroy();
		}
	}
}