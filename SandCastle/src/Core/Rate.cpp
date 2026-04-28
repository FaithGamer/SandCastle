#include "pch.h"
#include "SandCastle/Core/Rate.h"
#define ABSOLUTE_MAX 10000
namespace SandCastle
{
	Rate::Rate(double period, size_t sampleMax, double autoMaxPeriod, double rateUpdatePeriod) :
		m_period(period),
		m_sampleMax(sampleMax),
		m_sampleMin(sampleMax),
		m_autoMaxPeriod(autoMaxPeriod),
		m_rateUpdatePeriod(rateUpdatePeriod)
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
		if (m_autoMaxPeriod > 0.)
		{
			m_autoMaxTimer += (double)delta;
			if (m_autoMaxTimer >= m_autoMaxPeriod)
			{
				size_t newMax = std::max(m_autoMaxCount, m_sampleMin);
				newMax = std::min(newMax, (size_t)ABSOLUTE_MAX);
				if (newMax != m_sampleMax)
				{
					m_sampleMax = newMax;
					while (m_samples.size() > m_sampleMax)
					{
						m_sampleSum -= m_samples.front();
						m_samples.pop_front();
					}
				}
				m_autoMaxCount = 0;
				m_autoMaxTimer = 0.;
			}
		}
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
		if (m_rateUpdatePeriod > 0.)
		{
			m_rateUpdateTimer += (double)delta;
			if (m_rateUpdateTimer >= m_rateUpdatePeriod)
			{
				if (m_rateDirty)
				{
					Compute();
				}
				m_rateUpdateTimer = 0.;
			}
		}
	}
	void Rate::SetSampleMax(size_t max)
	{
		m_samples.clear();
		m_sampleSum = 0.;
		m_sampleMax = max;
	}
	void Rate::SetAutoMaxPeriod(double period)
	{
		m_autoMaxPeriod = period;
		m_autoMaxTimer = 0.;
		m_autoMaxCount = 0;
	}
	void Rate::SetRateUpdatePeriod(double period)
	{
		m_rateUpdatePeriod = period;
		m_rateUpdateTimer = 0.;
	}

	void Rate::Compute()
	{
		if (m_sampleSum > 0. && m_samples.size() > 0)
			rate = m_sampleSum / (double)m_samples.size();
		else
			rate = 0;
		m_rateDirty = false;
	}

	void Rate::PushSample(double sample)
	{
		m_samples.push_back(sample);
		m_sampleSum += sample;
		++m_autoMaxCount;
		if (m_samples.size() > m_sampleMax)
		{
			m_sampleSum -= m_samples.front();
			m_samples.pop_front();
		}
		if (m_rateUpdatePeriod > 0.)
		{
			m_rateDirty = true;
		}
		else
		{
			Compute();
		}
	}
}