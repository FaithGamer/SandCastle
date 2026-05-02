#include "pch.h"
#include "Roaming.h"
#include <windows.h>
#include <shlobj.h>
#include <algorithm>
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	namespace
	{
		String s_appName;

		String LocalLowFolder()
		{
			char path[MAX_PATH];
			String str;
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
			{
				str = path;
				for (auto& c : str)
					if (c == '\\') c = '/';
				str += "Low/";
			}
			else
			{
				LOG_ERROR("Cannot generate roaming path.");
			}
			return str;
		}

		String SanitizedAppName()
		{
			String s = s_appName;
			s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
			return s;
		}
	}

	void SetRoamingApp(const String& appName)
	{
		s_appName = appName;
	}

	String Roaming(const String& subPath)
	{
		String base = LocalLowFolder() + SanitizedAppName();
		if (!base.empty() && base.back() != '/')
			base += '/';
		return base + subPath;
	}
}
