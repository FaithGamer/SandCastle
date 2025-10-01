#pragma once

#include "SandCastle/Core/Math.h"
#include <deque>

namespace SandCastle
{
	class Rate
	{
	public:
		Rate() = default;
		Rate(double period, size_t sampleMax = 100) :
			m_period(period),
			m_sampleMax(sampleMax)
		{ }

		void Tick(double amount)
		{
			m_ticks += amount;
		}
		void Update(float delta)
		{
			m_time += (double)delta;
			if (m_ticks > 0)
			{
				PushSample(m_ticks / m_time * m_period);
				m_ticks = 0.;
				m_time = 0.;
			}
			else if (m_time > 1.)
			{
				rate -= rate * 0.5f * delta;
				if (rate < 0.) rate = 0.;
			}
		}
		void SetSampleMax(size_t max)
		{
			m_samples.clear();
			m_sampleSum = 0.;
			m_sampleMax = max;
		}
		double rate = 0.;

	private:
		void PushSample(double sample)
		{
			m_samples.push_back(sample);
			m_sampleSum += sample;
			if (m_samples.size() > m_sampleMax)
			{
				m_sampleSum -= m_samples.front();
				m_samples.pop_front();
			}
			rate = m_sampleSum / (double)m_samples.size();
		}
		double m_period = 1.;
		double m_time = 0.f;
		double m_ticks = 0.;
		std::deque<double> m_samples;
		double m_sampleSum = 0.;
		size_t m_sampleMax = 100;
	};
}
