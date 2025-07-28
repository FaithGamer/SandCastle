#include "pch.h"
#include "SandCastle/EngineParameters.h"

namespace SandCastle
{
	EngineParameters::EngineParameters()
		:
		appName("SandCastle Application"),
		startupWindowResolution(Vec2u(1280, 720)),
		enableImGui(true),
		fixedUpdateTimeStep(0.01f),
		fullscreen(false)
	{

	}
	EngineParameters::EngineParameters(Serialized settings)
	{
		Deserialize(settings);
	}

	void EngineParameters::Deserialize(Serialized& settings)
	{
		appName = settings.GetString("app_name");

		startupWindowResolution.x = settings.GetArray<int>("resolution")[0];
		startupWindowResolution.y = settings.GetArray<int>("resolution")[1];
		fullscreen = settings.GetBool("fullscreen");
		enableImGui = settings.GetBool("enable_imgui");
		fixedUpdateTimeStep = settings.GetFloat("fixed_time_step");

	}

	Serialized EngineParameters::Serialize()
	{
		Serialized s;
		s["app_name"] = "SandCastle Application";
		s["resolution"].push_back(startupWindowResolution.x);
		s["resolution"].push_back(startupWindowResolution.y);
		s["fullscreen"] = fullscreen;
		s["enable_imgui"] = enableImGui;
		s["fixed_time_step"] = (float)fixedUpdateTimeStep;

		return s;
	}
}