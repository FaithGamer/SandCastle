#pragma once

#include <vector>
#include "SandCastle/Core/Time.h"
#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	class Profiling
	{
	public:

		static void StartClock(String name);
		static void StopClock(String name);
		static void LogAllClocks();
		static float GetLastCycleMs(String name);

	public:
		struct ProfilingClock
		{
			Clock clock;
			Time lastCycle;
		};

		static std::unordered_map<String, ProfilingClock> clocks;
	private:

	};
}

#ifdef SC_PROFILING
#define START_PROFILING(clockName) Profiling::StartClock(clockName)
#define STOP_PROFILING(clockName) Profiling::StopClock(clockName)
#else
#define START_PROFILING(clockName)
#define STOP_PROFILING(clockName)
#endif
