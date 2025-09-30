#pragma once
#include "SandCastle/Core/std_macros.h"
#include <vector>
namespace SandCastle
{
	typedef std::string Version;
	class Versioning
	{
	public:
		static void Push(Version version);
		static Version Get();
		static std::vector<Version> GetAll();
	private:
		static std::vector<Version> versions;
		static Version version;
	};
}