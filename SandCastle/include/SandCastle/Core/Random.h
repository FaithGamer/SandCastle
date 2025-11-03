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
		double Range(double min, double max);
		template<typename T>
		T PickAndRemove(std::vector<T>& container)
		{
			int64_t r = Range((int64_t)0, (int64_t)(container.size() - 1));
			T ret = container[r];
			container[r] = container.back();
			container.pop_back();
			return ret;
		}
		template<typename T>
		T Pick(const std::vector<T>& container)
		{
			int64_t r = Range((int64_t)0, (int64_t)(container.size() - 1));
			return container[r];
		}
	}
}