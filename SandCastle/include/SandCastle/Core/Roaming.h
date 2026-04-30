#pragma once

#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	/// @brief Helper for resolving paths inside the per-user roaming/save folder
	/// (e.g. %APPDATA% on Windows). Register named subfolders once with AddPath()
	/// then resolve them anywhere with GetPath().
	class Roaming
	{
	public:
		/// @brief Register a named path relative to the roaming folder.
		static void AddPath(const String& id, const String& path);
		/// @brief Get the absolute path previously registered under id.
		static String GetPath(const String& id);
		/// @brief Absolute path of the application's roaming/save folder.
		static std::string RoamingFolder();
	private:
		static std::unordered_map<String, String> paths;
	};
}

