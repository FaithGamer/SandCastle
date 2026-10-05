#include "pch.h"

#include "SandCastle/Tools/TextureHotReload.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	void TextureHotReload::OnEvent(const SDL_Event& event)
	{
#ifndef SANDCASTLE_DISTRIB
		if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat || event.key.scancode != SDL_SCANCODE_F5)
			return;
		LOG_INFO("Hot reloading textures...");
		Assets::Instance()->HotReload();
		LOG_INFO("Textures reloaded.");
#endif
	}
}
