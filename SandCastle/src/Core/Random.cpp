#include "pch.h"
#include "SandCastle/Core/Random.h"

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
			std::uniform_int_distribution<int64_t> dist((int64_t)(min * 10000), (int64_t)(max * 10000));
			return (float)dist(generator) * 0.0001f;
		}
	}
}