#pragma once

#include <vector>
#include "SandCastle/Core/Time.h"
#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	/// @brief Lightweight named-clock profiler.
	/// Pair StartClock / StopClock around a region; LogAllClocks logs the last cycle
	/// duration of every named clock. Use the START_PROFILING/STOP_PROFILING macros
	/// if you want the calls compiled out unless SC_PROFILING is defined.
	class Profiling
	{
	public:

		/// @brief Start (or restart) the named clock.
		static void StartClock(String name);
		/// @brief Stop the named clock and store its elapsed time as last cycle.
		static void StopClock(String name);
		/// @brief Log the last-cycle duration of every clock to the engine logger.
		static void LogAllClocks();
		/// @brief Return the last-cycle duration of the named clock, in milliseconds.
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
/// @brief Begin a named profiler scope. Compiled out unless SC_PROFILING is defined.
#define START_PROFILING(clockName) Profiling::StartClock(clockName)
/// @brief End a named profiler scope. Compiled out unless SC_PROFILING is defined.
#define STOP_PROFILING(clockName) Profiling::StopClock(clockName)
#else
#define START_PROFILING(clockName)
#define STOP_PROFILING(clockName)
#endif
