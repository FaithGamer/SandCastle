#include "pch.h"
#include "SandCastle/Core/Profiling.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{

	std::unordered_map<String, Profiling::ProfilingClock> Profiling::clocks;

	void SandCastle::Profiling::StartClock(String name)

	{
		auto clock_it = clocks.find(name);
		if (clock_it == clocks.end())
		{
			auto it = clocks.insert(std::make_pair(name, ProfilingClock()));
			it.first->second.clock.Restart();
		}
		else
		{
			clock_it->second.clock.Restart();
		}
	}

	void SandCastle::Profiling::StopClock(String name) 
	{
		auto clock_it = clocks.find(name);
		if (clock_it == clocks.end())
		{
			LOG_WARN("StopClock: No profiling clock with the name {0}", name);
			return;
		}
		clock_it->second.lastCycle = clock_it->second.clock.GetElapsed();
	}

	void Profiling::LogAllClocks()
	{
		for (auto& clock_kvp : clocks)
		{
			auto& clock = clock_kvp.second;
			String name = clock_kvp.first;
			float ms = clock.lastCycle * 1000.f;
			LOG_INFO("{0}: {1}ms", name, ms);
		}
	}

	float Profiling::GetLastCycleMs(String name)
	{
		auto clock_it = clocks.find(name);
		if (clock_it == clocks.end())
		{
			LOG_WARN("GetLastCycle: No profiling clock with the name {0}", name);
			return 0.f;
		}
		return clock_it->second.lastCycle * 1000.f;
	}

}