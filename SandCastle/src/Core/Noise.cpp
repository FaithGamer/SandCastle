#include "pch.h"
#include "SandCastle/Core/Noise.h"

namespace SandCastle
{
	Noise::Noise() : m_frequency(0.5f)
	{
	}
	double Noise::Get(double x, double y)
	{
		return m_generator.noise2D_01(x * m_frequency, y * m_frequency);
	}
	void Noise::SetFrequency(double frequency)
	{
		m_frequency = frequency;
	}
	void Noise::Seed(unsigned int seed)
	{
		m_generator.reseed(seed);
	}
	double Noise::Noise2D(double x, double y, double frequency)
	{
		static siv::PerlinNoise generator;
		return generator.noise2D_01(x * frequency, y * frequency);
	}

}
