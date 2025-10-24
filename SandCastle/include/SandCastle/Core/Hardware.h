#pragma once
#include <fstream>
#include <string>
#include <algorithm>

namespace SandCastle
{
	class Hardware
	{
	public:
		static bool IsSteamDeck()
		{
			// Proton maps the Linux root as Z:
			bool vendorValve = fileContains("Z:\\sys\\class\\dmi\\id\\sys_vendor", "Valve");
			bool isJupiter = fileContains("Z:\\sys\\class\\dmi\\id\\product_name", "Jupiter");
			bool isGalileo = fileContains("Z:\\sys\\class\\dmi\\id\\product_name", "Galileo");
			return (IsDeckEnv() && (vendorValve || isJupiter || isGalileo));
		}
	private:
		static bool fileContains(const char* path, const char* needle)
		{
			std::ifstream f(path);
			if (!f) return false;
			std::string s((std::istreambuf_iterator<char>(f)), {});
			std::string n = needle;
			std::transform(s.begin(), s.end(), s.begin(), ::tolower);
			std::transform(n.begin(), n.end(), n.begin(), ::tolower);
			return s.find(n) != std::string::npos;
		}
		static bool IsDeckEnv()
		{
			char* buffer = nullptr;
			size_t size = 0;
			if (_dupenv_s(&buffer, &size, "SteamDeck") == 0 && buffer != nullptr)
			{
				std::string val(buffer);
				free(buffer); // must free memory allocated by _dupenv_s
				return val == "1";
			}
			return false;
		}
	};
}