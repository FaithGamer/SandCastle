#include "pch.h"
#include "SandCastle/Core/Random.h"
#include "SandCastle/Core/Int128.h"

namespace SandCastle
{
	namespace Random
	{
		static std::default_random_engine generator;
		void Seed(unsigned int seed)
		{
			generator.seed(seed);
		}
		int Range(int min, int max)
		{
			//static std::default_random_engine generator;

			std::uniform_int_distribution<int> dist(min, max);
			return dist(generator);
		}

		uint64_t Range(uint64_t min, uint64_t max)
		{
			//static std::default_random_engine generator;

			std::uniform_int_distribution<uint64_t> dist(min, max);
			return dist(generator);
		}

		int64_t Range(int64_t min, int64_t max)
		{
			//static std::default_random_engine generator;

			std::uniform_int_distribution<int64_t> dist(min, max);
			return dist(generator);
		}

		/// @brief /!\ max precision 0.0001f
		/// @param min
		/// @param max
		/// @return random range between min and max.
		float Range(float min, float max)
		{
			//static std::default_random_engine generator;
			std::uniform_int_distribution<int64_t> dist((int64_t)(min * 10000.), (int64_t)(max * 10000.));
			return (float)dist(generator) * 0.0001;
		}
		/// @brief /!\ max precision 0.0001
		/// @param min
		/// @param max
		/// @return random range between min and max.
		double Range(double min, double max)
		{
			//static std::default_random_engine generator;
			std::uniform_int_distribution<int64_t> dist((int64_t)(min * 10000.), (int64_t)(max * 10000.));
			return (double)dist(generator) * 0.0001;
		}

		Int128 Range(Int128 min, Int128 max)
		{
			Int128 range = max - min;

			// Fast path: range fits in uint64_t (high word is zero)
			if (range.high == 0)
			{
				uint64_t r = Range(static_cast<uint64_t>(0), range.low);
				return min + static_cast<Int128>(r);
			}

			// General: rejection-sample a value in [0, range].
			// Pick hi in [0, range.high] and lo in [0, UINT64_MAX].
			// Only reject when hi == range.high and lo > range.low (rare).
			uint64_t hiMax = static_cast<uint64_t>(range.high);
			while (true)
			{
				uint64_t hi = Range(static_cast<uint64_t>(0), hiMax);
				uint64_t lo = Range(static_cast<uint64_t>(0), UINT64_MAX);
				Int128 candidate(static_cast<int64_t>(hi), lo);
				if (candidate <= range)
					return min + candidate;
			}
		}
	}
}
