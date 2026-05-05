#pragma once

#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	/// @brief Open a URL or URI in the system's default handler
	/// (browser for http(s)://, mail client for mailto:, file manager
	/// for file:///, etc.). Returns true if the OS reported the launch
	/// succeeded; false on failure (call SDL_GetError for details).
	bool OpenURL(const String& url);
}
