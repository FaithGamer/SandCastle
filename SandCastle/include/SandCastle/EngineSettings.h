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
	struct EngineSettings : public Serializable
	{
		EngineSettings();
		EngineSettings(Serialized settings);
		void Deserialize(Serialized& parameters) override;
		Serialized Serialize() override;

		std::string appName;
		Vec2u startupWindowResolution;
		bool enableImGui;
		Time fixedUpdateTimeStep;
		bool fullscreen;
		TextureImportSettings textureImport;

	};
}