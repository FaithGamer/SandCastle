#pragma once

#define SandCastle_IMGUI

namespace SandCastle
{
	class Engine
	{
	public:
		static void Init(bool logging = true);
		static void Launch();
		static void Stop();
	private:
		static bool play;
	};
}
