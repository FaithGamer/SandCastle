#pragma once

#include <SandCastle/SandCastle.h>
#include <SandCastle/Tools/SpriteExport.h>

using namespace SandCastle;

class SpriteExportSystem : public System
{
public:
	int GetUsedMethod() override { return ImGui; }
	void OnImGui() override { 
		/*struct SpriteExportConfig
		{
			/// Path to the Aseprite executable.
			std::string asepriteExe = "C:/Program Files/Aseprite-v1.3.13-x64/Aseprite.exe";
			/// Root folder where source .aseprite files live (used as the starting
			/// directory for the Open... file picker).
			std::string asepriteDir = "art";
			/// Output folder for the generated .png spritesheet and .texture file.
			std::string textureDir = "assets/textures";
			/// Output folder for the generated .anim files (one per tag).
			std::string animationDir = "assets/animations";
			/// Pixels of padding inserted on each side of every frame in the sheet
			/// (mirrors Aseprite's "inner padding" export option).
			int         innerPadding = 2;
			/// Maximum columns in the exported spritesheet. 0 = unlimited.
			int         maxColumns = 0;
			/// Sprite origin in normalized [0..1] coordinates (written to .texture).
			/// (0,0) = top-left, (0.5,0.5) = center, (1,1) = bottom-right.
			float       originX = 0.0f;
			float       originY = 0.0f;
		};*/
		SpriteExportConfig cfg;
		cfg.asepriteDir = "c:/dev/TheReclaim/art";
		cfg.textureDir = "c:/dev/TheReclaim/assets/textures";
		cfg.animationDir = "c:/dev/TheReclaim/assets/animations";
		ShowSpriteExport(cfg); 
	}
};

void SpriteExportTool()
{
	EngineSettings settings;
	settings.appName = "Sprite Export Tool";
	settings.startupWindowResolution = Vec2u(640, 420);
	settings.frameCap = 10;

	Engine::Init(settings);
	Systems::Push<SpriteExportSystem>();
	Engine::Launch();
}
