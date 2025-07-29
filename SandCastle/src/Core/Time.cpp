#include "pch.h"
#include "SandCastle/Core/Time.h"



namespace SandCastle
{
	Time Time::delta = 0;
	Time Time::fixedDelta = 0.1f;
	float Time::timeScale = 1.f;
	////////////////////////////
	/// Time ///////////////////
	////////////////////////////
	Time Time::Delta()
	{
		return (float)delta * timeScale;
	}
	Time Time::FixedDelta()
	{
		return (float)fixedDelta * timeScale;
	}

	Time Time::UnscaledDelta()
	{
		return delta;
	}

	Time Time::UnscaledFixedDelta()
	{
		return fixedDelta;
	}

	Time::Time() : m_microseconds(std::chrono::microseconds::zero())
	{

	}

	template <typename rep, typename ratio>
	Time::Time(const std::chrono::duration<rep, ratio>& duration)
		: m_microseconds(std::chrono::duration_cast<std::chrono::microseconds>(duration))
	{

	}

	Time::Time(float seconds) : m_microseconds((int64_t)(seconds * 1000000))
	{

	}

	template <typename chrono_duration>
	chrono_duration Time::GetDuration() const
	{
		return std::chrono::duration_cast<chrono_duration>(m_microseconds);
	}

	Time::operator float() const
	{
		return (float)m_microseconds.count() / 1000000.f;
	}


	Time& operator+=(Time& l, const Time& r)
	{
		return l = Time(l.m_microseconds + r.m_microseconds);
	}
	Time& operator-=(Time& l, const Time& r)
	{
		return l = Time(l.m_microseconds - r.m_microseconds);
	}

	Time operator+(const Time& l, const Time& r)
	{
		return Time(l.m_microseconds + r.m_microseconds);
	}
	Time operator-(const Time& l, const Time& r)
	{
		return Time(l.m_microseconds - r.m_microseconds);
	}

	////////////////////////////
	/// Clock //////////////////
	////////////////////////////

	Clock::Clock() :
		m_start(std::chrono::high_resolution_clock::now())
	{

	}

	Time Clock::GetElapsed() const
	{
		return Time(std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::high_resolution_clock::now() - m_start)));
	}

	Time Clock::Restart()
	{
		Time t = GetElapsed();
		m_start = std::chrono::high_resolution_clock::now();
		return t;
	}
}