#pragma once
#include "SandCastle/Core/Int128.h"

namespace SandCastle
{
	/// @brief Engine-wide pseudo-random number generator with uniform Range() across numeric types.
	/// Seed() controls reproducibility. Range() bounds are inclusive on both ends.
	namespace Random
	{
		/// @brief Seed the global RNG.
		void Seed(unsigned int seed);
		/// @brief Uniform integer in [min, max] (inclusive).
		int Range(int min, int max);
		/// @brief Uniform unsigned 64-bit integer in [min, max] (inclusive).
		uint64_t Range(uint64_t min, uint64_t max);
		/// @brief Uniform signed 64-bit integer in [min, max] (inclusive).
		int64_t Range(int64_t min, int64_t max);
		/// @brief Uniform float in [min, max].
		float Range(float min, float max);
		/// @brief Uniform double in [min, max].
		double Range(double min, double max);
		/// @brief Uniform 128-bit integer in [min, max] (inclusive).
		Int128 Range(Int128 min, Int128 max);
		/// @brief Pick a random element from container, remove it (swap-and-pop), return it. O(1) order-altering.
		template<typename T>
		T PickAndRemove(std::vector<T>& container)
		{
			int64_t r = Range((int64_t)0, (int64_t)(container.size() - 1));
			T ret = container[r];
			container[r] = container.back();
			container.pop_back();
			return ret;
		}
		/// @brief Return a random element of container without modifying it.
		template<typename T>
		const T& Pick(const std::vector<T>& container)
		{
			int64_t r = Range((int64_t)0, (int64_t)(container.size() - 1));
			return container[r];
		}
	}
}
