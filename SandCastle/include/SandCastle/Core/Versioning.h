#pragma once
#include "SandCastle/Core/std_macros.h"
#include <vector>
namespace SandCastle
{
	/// @brief Free-form version string (e.g. "1.4.2-rc1").
	typedef std::string Version;
	/// @brief Stack of version strings for the running build.
	/// Useful for save-file migration: push a Version each time the format changes,
	/// then check Get() / GetAll() during deserialization.
	class Versioning
	{
	public:
		/// @brief Push a new current version on the stack.
		static void Push(Version version);
		/// @brief Most recently pushed version.
		static Version Get();
		/// @brief Every version pushed so far, in chronological order.
		static std::vector<Version> GetAll();
	private:
		static std::vector<Version> versions;
		static Version version;
	};
}