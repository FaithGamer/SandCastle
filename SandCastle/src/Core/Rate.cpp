#include "pch.h"
#include "SandCastle/Core/Rate.h"

namespace SandCastle
{
	Rate::Rate(double period, size_t sampleMax) :
		m_period(period),
		m_sampleMax(sampleMax)
	{
	}

	void Rate::Tick(double amount)
	{
		m_ticks += amount;
	}
	void Rate::Update(float delta)
	{
		m_time += (double)delta;
		m_decrTime += (double)delta;
		if (m_ticks > 0)
		{
			PushSample(m_ticks / m_time * m_period);
			m_ticks = 0.;
			m_lastInterval = m_time;
			m_time = 0.;
			m_decrTime = 0.;
		}
		else if (m_decrTime > std::min(minRate, m_lastInterval*2.))
		{
			PushSample(0.);
			m_decrTime = 0.;
		}
	}
	void Rate::SetSampleMax(size_t max)
	{
		m_samples.clear();
		m_sampleSum = 0.;
		m_sampleMax = max;
	}

	void Rate::PushSample(double sample)
	{
		m_samples.push_back(sample);
		m_sampleSum += sample;
		if (m_samples.size() > m_sampleMax)
		{
			m_sampleSum -= m_samples.front();
			m_samples.pop_front();
		}
		if (m_sampleSum > 0. && m_samples.size() > 0)
			rate = m_sampleSum / (double)m_samples.size();
		else
			rate = 0;

	}
}