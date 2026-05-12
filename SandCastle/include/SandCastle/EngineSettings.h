#pragma once
#include <string>
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/Time.h"
#include "SandCastle/Core/Serialization.h"
#include "SandCastle/Render/TextureSettings.h"

namespace SandCastle
{
	//To do: enable serialization/deserialization
	/// @brief Parameters consumed by Engine::Init() to bootstrap the runtime.
	/// Holds the application identity (used for the window title and the
	/// roaming/save folder), where assets live on disk, the startup window
	/// resolution and fullscreen mode, the FixedUpdate cadence, and the
	/// default texture import options applied to every loaded texture.
	struct EngineSettings : public Serializable
	{
		EngineSettings();
		EngineSettings(Serialized settings);
		void Deserialize(Serialized& parameters) override;
		Serialized Serialize() override;

		std::string appName;
		std::string assetFolder;
		Vec2u startupWindowResolution;
		Time fixedUpdateTimeStep;
		bool fullscreen;
		TextureImportSettings textureImport;
		String defaultLang;
		/// Max frames per second. 0 = unlimited.
		int frameCap = 0;
	};
}