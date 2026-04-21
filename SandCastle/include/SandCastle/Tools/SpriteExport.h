#pragma once

#include <string>

namespace SandCastle
{
	/// @brief Configuration for the sprite exporter tool.
	/// All fields have sensible defaults, so `ShowSpriteExport()` can be called
	/// with no arguments for zero-effort usage.
	struct SpriteExportConfig
	{
		/// Path to the Aseprite executable.
		std::string asepriteExe  = "C:/Program Files/Aseprite-v1.3.13-x64/Aseprite.exe";
		/// Root folder where source .aseprite files live (used as the starting
		/// directory for the Open... file picker).
		std::string asepriteDir  = "art";
		/// Output folder for the generated .png spritesheet and .texture file.
		std::string textureDir   = "assets/textures";
		/// Output folder for the generated .anim files (one per tag).
		std::string animationDir = "assets/animations";
		/// Pixels of padding inserted on each side of every frame in the sheet
		/// (mirrors Aseprite's "inner padding" export option).
		int         innerPadding = 2;
		/// Maximum columns in the exported spritesheet. 0 = unlimited.
		int         maxColumns   = 0;
	};

	/// @brief Renders an inline ImGui widget that exports an Aseprite file
	/// into a .png spritesheet + .texture file (+ one .anim per tag).
	///
	/// The config's values are used as initial defaults on the first call;
	/// subsequent calls keep the user's edits to the in-UI fields.
	///
	/// Draws at the current ImGui cursor (no window is created), so the caller
	/// decides where to place it.
	///
	/// Requires the `aseprite` CLI.
	void ShowSpriteExport(const SpriteExportConfig& cfg = SpriteExportConfig{});
}
