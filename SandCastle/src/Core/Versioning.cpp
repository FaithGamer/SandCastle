#include "pch.h"
#include "SandCastle/Core/Versioning.h"
#include "SandCastle/Engine.h"
#include "SandCastle/Core/Log.h"
namespace SandCastle
{
	Version Versioning::version = "1.0";
	std::vector<Version> Versioning::versions = {};

	String Versioning::Get()
	{
		return version;
	}
	std::vector<String> Versioning::GetAll()
	{
		return versions;
	}
	void Versioning::Push(Version ver)
	{
		assert(!Engine::IsInit()); // do not push a version after engine has been initialized.
		versions.emplace_back(ver);
		version = ver;
	}
}