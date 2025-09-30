#include "pch.h"
#include "Roaming.h"
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include "SandCastle/Core/Log.h"
#include "SandCastle/Engine.h"

namespace SandCastle
{
	std::unordered_map<String, String> Roaming::paths;
	void Roaming::AddPath(const String& id, const String& path)
	{
		assert(!Engine::IsInit());
		paths.insert(std::make_pair(id, RoamingFolder() + path));
	}

	String Roaming::GetPath(const String& id)
	{
		auto f_it = paths.find(id);
		if (f_it == paths.end())
		{
			LOG_ERROR("Roaming path {0}, doesn't exists", id);
			return "";
		}
		return f_it->second;
	}

	std::string Roaming::RoamingFolder()
	{
		char path[999];
		std::string str = "";
		if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
			str = std::string(path);
			for (auto& c : str)
			{
				if (c == '\\')
					c = '/';
			}
			str += "Low/";
		}
		else
		{
			LOG_ERROR("Cannot generate roaming path.");
		}
		return str;
	}
}