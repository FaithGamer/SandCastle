#pragma once

union SDL_Event;

namespace SandCastle
{
	/// @brief Dev tool: pressing F5 calls Assets::HotReload(), re-uploading every
	/// texture from disk in place (existing Texture/Sprite pointers stay valid).
	/// Fed by the engine event loop; compiled out in Distrib builds.
	class TextureHotReload
	{
	public:
		/// @brief Trigger a texture reload on an F5 key press (key repeats ignored).
		/// Never consumes the event, so the game still receives F5.
		static void OnEvent(const SDL_Event& event);
	};
}
