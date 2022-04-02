#include "Engine.h"
#include "Graphics/Vulkan/VulkanBase.h"
#include "ClientScene.h"

namespace zyh
{
	Engine* GEngine = new Engine();
	Engine::Engine()
	{
		Scene = new ClientScene();
		Vulkan = new VulkanBase();
	}

	Engine::~Engine()
	{
		delete Scene;
		delete Vulkan;
	}

	void Engine::Run()
	{
		Initialize();
		while (true)
		{
			Tick();
		}
		CleanUp();
	}

	void Engine::Initialize()
	{
		Scene->Initialize();
		Vulkan->Initialize(GetModuleHandle(0));
	}

	void Engine::Tick()
	{
		Scene->Tick();
		Vulkan->Tick();
	}

	void Engine::CleanUp()
	{
		Scene->CleanUp();
		Vulkan->CleanUp();
	}

}
