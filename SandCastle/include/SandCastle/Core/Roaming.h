#pragma once

#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	/// @brief Resolve a path inside the per-user roaming/save folder
	/// for this application (e.g. %LOCALAPPDATA%Low/<appName>/<subPath> on Windows).
	/// Spaces in the application name are stripped when forming the folder name.
	/// The application name is captured from EngineSettings::appName at Engine::Init.
	String Roaming(const String& subPath);

	/// @brief Set the application name used by Roaming(). Called by Engine::Init.
	void SetRoamingApp(const String& appName);
}
