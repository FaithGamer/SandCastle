#pragma once

namespace SandCastle
{
	namespace Random
	{
		void Seed(unsigned int seed);
		int Range(int min, int max);
		uint64_t Range(uint64_t min, uint64_t max);
		int64_t Range(int64_t min, int64_t max);
		float Range(float min, float max);
	}
}
