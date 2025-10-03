#pragma once

#include "SandCastle/Core/Math.h"
#include <deque>

namespace SandCastle
{
	class Rate
	{
	public:
		Rate() = default;
		Rate(double period, size_t sampleMax = 100);

		void Tick(double amount);
		void Update(float delta);
		void SetSampleMax(size_t max);
		double rate = 0.;
		double minRate = 1.;

	private:
		void PushSample(double sample);
		double m_lastInterval = 0.;
		double m_period = 1.;
		double m_time = 0.f;
		double m_decrTime = 0.f;
		double m_ticks = 0.;
		std::deque<double> m_samples;
		double m_sampleSum = 0.;
		size_t m_sampleMax = 100;
	};
}
