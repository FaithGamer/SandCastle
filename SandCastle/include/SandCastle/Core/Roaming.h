#pragma once

#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	class Roaming
	{
	public:
		static void AddPath(const String& id, const String& path);
		static String GetPath(const String& id);
		static std::string RoamingFolder();
	private:
		static std::unordered_map<String, String> paths;
	};
}
