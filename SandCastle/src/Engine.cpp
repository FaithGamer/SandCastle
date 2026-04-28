#include "pch.h"

#include "SandCastle/Engine.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/EngineSettings.h"
#include "SandCastle/Input/Inputs.h"
#include "SandCastle/Internal/ImGuiLoader.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/AnimationSystem.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Render/SpriteRenderSystem.h"
#include "SandCastle/Render/LineRendererSystem.h"
#include "SandCastle/Render/WireRenderSystem.h"
#include "SandCastle/Render/ParticleSystem.h"
#include "SandCastle/Physics/PhysicsSystem.h"
#include "SandCastle/Physics/Physics.h"
#include "SandCastle/Audio/Audio.h"
#include "SandCastle/UI/Ui.h"
#include "SandCastle/Core/Versioning.h"

namespace SandCastle
{
	bool Engine::play = true;
	bool Engine::init = false;

	void Engine::Init(EngineSettings settings)
	{
		Log::Instance()->Init();

		LOG_INFO("Engine start.");

		TextureImportSettings::defaultSettings = settings.textureImport;

		LOG_INFO("Creating window...");
		String windowName = settings.appName + " - " + Versioning::Get();
		Window::Instance()->Init(windowName, settings.startupWindowResolution);
		Window::SetFullScreen(settings.fullscreen);
		LOG_INFO("Initializing context...");
		Renderer2D::Instance()->Init();
		LOG_INFO("Initializing audio...");
		Audio::Instance()->Init();
		LOG_INFO("Loading assets...");
		Assets::Instance()->Init(settings.assetFolder, settings.defaultLang);
		Audio::Instance()->PostAssetInit();
		LOG_INFO("Initializing renderer...");
		Renderer2D::Instance()->PostAssetInit();
		Renderer2D::AddLayer("DebugLayer");
		LOG_INFO("Initializing Physics...");
		Physics::Instance();
		LOG_INFO("Loading UI...");
		Inputs::Instance();
		Ui::Instance();
		LOG_INFO("Creating world...");
		auto system = Systems::Instance();
		system->Init();
		Systems::SetFixedUpdateTime(settings.fixedUpdateTimeStep);
#ifdef SC_IMGUI
		ImGuiLoader::LoadImGui(Window::GetSDLWindow(), Window::GetRenderContext());
#endif

		//Default systems
		Systems::Push<SpriteRenderSystem>();
		Systems::Push<LineRendererSystem>();
		Systems::Push<WireRenderSystem>();
		Systems::Push<AnimationSystem>();
		Systems::Push<PhysicsSystem>();
		Systems::Push<ParticleSystem>();

		init = true;
	}

	void Engine::Init(const String& settingsPath)
	{
		Serialized settingsJson(settingsPath);
		EngineSettings settings;
		if (settingsJson.HadLoadError())
		{
			LOG_WARN("Couldn't load engine parameters at path: {0}", settingsPath);
		}
		else
		{
			settings = EngineSettings(settingsJson);
		}
		Init(settings);
	}

	void Engine::Launch()
	{
		play = true;
		while (play)
		{
			Systems::Instance()->Update();
		}

		Renderer2D::Instance()->Wait();

		Inputs::Kill();
		Systems::Kill();
		Ui::Kill();
		Renderer2D::Kill();
		Window::Kill();
#ifdef SC_IMGUI
		ImGuiLoader::ExitImGui();
#endif
	}

	void Engine::Stop()
	{
		play = false;
	}
	bool Engine::IsInit()
	{
		return init;
	}
}

