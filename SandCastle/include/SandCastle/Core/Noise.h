#pragma once

#include <PerlinNoise/PerlinNoise.hpp>

namespace SandCastle
{
	class Noise
	{
	public:
		Noise();
		/// @brief Get noise value for this coordinate
		/// @return noise value between 0 and 1
		double Get(double x, double y);
		/// @brief Set the noise frequency
		/// @param frequency range between 0 (big dense blocks) and 1 (little sparse blocks)
		void SetFrequency(double frequency);
		// @brief Set the persistence
		/// @param persistence, 1 = all octave contribute equally, higher than one, each subsequent octave contribute more
		void SetPersistence(double persistence);
		/// @brief Set the octave count
		/// @param octave number of noise passes. (higher value: finer details)
		void SetOctave(int octave);
		/// @brief Set the seed for RNG
		void Seed(unsigned int seed);
		/// @brief Generate noise with a static generator
		/// @return noise value between 0 and 1
		static double Noise2D(double x, double y, double frequency);
	private:
		siv::PerlinNoise m_generator;
		double m_frequency = 1.;
		double m_persistence = 1.;
		int m_octave = 1;
		

	};
}
