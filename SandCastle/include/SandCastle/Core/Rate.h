#pragma once

#include "SandCastle/Core/Math.h"
#include <deque>

namespace SandCastle
{
	/// @brief Smoothed event-rate tracker (events per `period` seconds).
	/// Call Tick() each time an event happens and Update(delta) every frame.
	/// `rate` ramps up/down based on the rolling sample window. Used by
	/// HighrateSound to crossfade loops based on how often a sound is triggered.
	class Rate
	{
	public:
		Rate() = default;
		/// @brief @param period Sampling window in seconds.
		/// @param sampleMax Max number of recent samples kept in the rolling window.
		/// @param autoMaxPeriod If > 0, sampleMax is auto-tuned over this period.
		/// @param rateUpdatePeriod Minimum interval between rate recomputations.
		Rate(double period, size_t sampleMax = 100, double autoMaxPeriod = 0., double rateUpdatePeriod = 0.);

		/// @brief Record one event of magnitude `amount` happening now.
		void Tick(double amount);
		/// @brief Advance internal timers, decay the rate when no Ticks come in.
		void Update(float delta);
		/// @brief Override the size of the rolling sample window.
		void SetSampleMax(size_t max);
		/// @brief Enable automatic adjustment of sampleMax over the given period (0 disables).
		void SetAutoMaxPeriod(double period);
		/// @brief Throttle rate recomputation to once per `period` seconds.
		void SetRateUpdatePeriod(double period);
		/// @brief Current smoothed rate (read-only for outside code).
		double rate = 0.;
		/// @brief Minimum rate clamp; rate never drops below this value.
		double minRate = 1.;

	private:
		void Compute();
		void PushSample(double sample);
		double m_lastInterval = 0.;
		double m_period = 1.;
		double m_time = 0.f;
		double m_decrTime = 0.f;
		double m_ticks = 0.;
		std::deque<double> m_samples;
		double m_sampleSum = 0.;
		size_t m_sampleMax = 100;
		size_t m_sampleMin = 100;
		double m_autoMaxPeriod = 0.;
		double m_autoMaxTimer = 0.;
		size_t m_autoMaxCount = 0;
		double m_rateUpdatePeriod = 0.;
		double m_rateUpdateTimer = 0.;
		bool m_rateDirty = false;
	};
}
