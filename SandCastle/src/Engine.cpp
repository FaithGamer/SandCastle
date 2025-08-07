#include "pch.h"

#include "SandCastle/Engine.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/EngineParameters.h"
#include "SandCastle/Input/Inputs.h"
#include "SandCastle/Internal/ImGuiLoader.h"
#include "SandCastle/Input/InputSystem.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/AnimationSystem.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/ECS/SpriteRenderSystem.h"
#include "SandCastle/ECS/LineRendererSystem.h"
#include "SandCastle/ECS/WireRenderSystem.h"
#include "SandCastle/Physics/PhysicsSystem.h"
#include "SandCastle/Physics/Physics.h"
#include "SandCastle/Audio/Audio.h"


namespace SandCastle
{
	bool Engine::play = true;

	void Engine::Init(bool logging)
	{
		Log::Instance()->Init(logging);

		LOG_INFO("Engine start.");
		EngineParameters params;
		Serialized paramsJson("assets/config/application.config");

		if (paramsJson.HadLoadError())
		{
			LOG_WARN("Couldn't load engine parameters, creating one with default values.");

			Serialized parametersWriteOnDisk = params.Serialize();
			parametersWriteOnDisk.WriteOnDisk("assets/config/application.config");
		}
		else
		{
			params = EngineParameters(paramsJson);
		}


		LOG_INFO("Loading window...");
		Window::Instance()->Init(params.appName, params.startupWindowResolution);
		Window::SetFullScreen(params.fullscreen);
		Renderer2D::Instance()->Init();
		Renderer2D::Instance()->Wait();
		Renderer2D::AddLayer("DebugLayer");
		LOG_INFO("Loading audio...");
		Audio::Instance()->Init();
		LOG_INFO("Loading assets...");
		Assets::Instance()->Init();
		LOG_INFO("Loading renderer...");
		LOG_INFO("Loading Physics...");
		Physics::Instance();
		LOG_INFO("Creating world...");
		auto system = Systems::Instance();
		system->Init();
		system->CreateWorld();
		Systems::SetFixedUpdateTime(params.fixedUpdateTimeStep);
#ifndef SANDCASTLE_DISTRIB
		LoadImGui(Window::GetSDLWindow(), Window::GetRenderContext());
#endif

		//Default systems
		Systems::Push<InputSystem>();
		Systems::Push<SpriteRenderSystem>();
		Systems::Push<LineRendererSystem>();
		Systems::Push<WireRenderSystem>();
		Systems::Push<AnimationSystem>();
		Systems::Push<PhysicsSystem>();
	}

	void Engine::Launch()
	{
		//Main loop
		play = true;
		while (play)
		{
			Systems::Instance()->Update();
		}

		//Deallocation
		Inputs::Kill();
		Systems::Kill();
		Window::Kill();
	}

	void Engine::Stop()
	{
		play = false;
	}
}

