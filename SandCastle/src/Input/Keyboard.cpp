#include "pch.h"
#include "SandCastle/Input/Keyboard.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	std::string KeycodeNameFromScancode(KeyScancode key)
	{
		if ((int)key < 0 || (int)key >= SDL_SCANCODE_COUNT)
		{
			LOG_ERROR("Trying to get the name of an out of bound Key Scancode: " + std::to_string((int)key));
			return "UnknowKey";
		}
		return std::string(SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)key, SDL_KMOD_NONE, true)));
	}
	std::string ScancodeName(KeyScancode key)
	{
		if ((int)key < 0 || (int)key >= SDL_SCANCODE_COUNT)
		{
			LOG_ERROR("Trying to get the name of an out of bound Key Scancode: " + std::to_string((int)key));
			return "UnknowKey";
		}
		return std::string(SDL_GetKeyName(SDL_SCANCODE_TO_KEYCODE((SDL_Scancode)key)));
	}
	KeyScancode KeyScancodeFromName(std::string name)
	{
		KeyScancode scancode = (KeyScancode)SDL_GetScancodeFromName(name.c_str());
		if (scancode == KeyScancode::Unknown)
		{
			LOG_ERROR("Key name unknown, " + LogSDLError(""));
		}
		return scancode;
	}
}