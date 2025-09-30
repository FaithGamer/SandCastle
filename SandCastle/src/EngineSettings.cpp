#include "pch.h"
#include "SandCastle/EngineSettings.h"

namespace SandCastle
{
	EngineSettings::EngineSettings()
		:
		appName("SandCastle Application"),
		startupWindowResolution(Vec2u(1280, 720)),
		fixedUpdateTimeStep(0.01f),
		fullscreen(false),
		textureImport(TextureImportSettings()),
		defaultLang(""),
		assetFolder("assets/")
	{

	}
	EngineSettings::EngineSettings(Serialized settings)
	{
		Deserialize(settings);
	}

	void EngineSettings::Deserialize(Serialized& settings)
	{
		appName = settings.GetString("app_name");

		startupWindowResolution.x = settings.GetArray<int>("resolution")[0];
		startupWindowResolution.y = settings.GetArray<int>("resolution")[1];
		fullscreen = settings.GetBool("fullscreen");
		fixedUpdateTimeStep = settings.GetFloat("fixed_time_step");
		auto texture = settings.GetObj("texture_import");
		textureImport.Deserialize(texture);
		defaultLang = settings.GetString("default_lang");
		assetFolder = settings.GetString("asset_folder");
	}

	Serialized EngineSettings::Serialize()
	{
		Serialized s;
		s["app_name"] = "SandCastle Application";
		s["resolution"].push_back(startupWindowResolution.x);
		s["resolution"].push_back(startupWindowResolution.y);
		s["fullscreen"] = fullscreen;
		s["fixed_time_step"] = (float)fixedUpdateTimeStep;
		s["texture_import"] = textureImport.Serialize();
		s["default_lang"] = defaultLang;
		s["asset_folder"] = assetFolder;

		return s;
	}
}