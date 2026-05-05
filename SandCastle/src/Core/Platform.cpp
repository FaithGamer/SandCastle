#include "pch.h"
#include "SandCastle/Core/Platform.h"
#include <SDL3/SDL_misc.h>

namespace SandCastle
{
	bool OpenURL(const String& url)
	{
		return SDL_OpenURL(url.c_str());
	}
}
