#pragma once
#include <chrono>

namespace SandCastle
{
	class Systems;

	/// @brief Microsecond-precision duration with implicit conversion to seconds (float).
	/// Use the static Delta()/FixedDelta() inside Update()/FixedUpdate() to step
	/// game logic. Unscaled* variants ignore Systems::SetTimeScale() (handy for UI).
	class Time
	{
	public:
		Time();

		/// @brief Build a Time from a duration expressed in seconds.
		Time(const float seconds);

		/// @brief Build a Time from any std::chrono::duration.
		template <typename rep, typename ratio>
		Time(const std::chrono::duration<rep, ratio>& duration);

		/// @brief Convert to a chrono duration (default: microseconds).
		template <typename chrono_duration = std::chrono::microseconds>
		chrono_duration GetDuration() const;

		/// @brief Implicit conversion to seconds as float.
		operator float() const;

		/// @brief Time elapsed since the previous Update(), affected by time scale.
		static Time Delta();
		/// @brief Fixed-step duration used by FixedUpdate(), affected by time scale.
		static Time FixedDelta();
		/// @brief Real-time delta, ignoring time scale.
		static Time UnscaledDelta();
		/// @brief Real fixed step, ignoring time scale.
		static Time UnscaledFixedDelta();

	private:
		friend Systems;
		static Time delta;
		static Time fixedDelta;
		static float timeScale;

		std::chrono::microseconds m_microseconds;

		friend Time& operator+=(Time& l, const Time& r);
		friend Time& operator-=(Time& l, const Time& r);

		friend Time operator+(const Time& l, const Time& r);
		friend Time operator-(const Time& l, const Time& r);
	};

	Time& operator+=(Time& l, const Time& r);
	Time& operator-=(Time& l, const Time& r);

	Time operator+(const Time& l, const Time& r);
	Time operator-(const Time& l, const Time& r);

	/// @brief Stopwatch built on std::chrono::high_resolution_clock.
	/// Construction starts the clock; use GetElapsed() to peek and Restart() to reset and read in one call.
	class Clock
	{

	public:
		Clock();
		/// @brief Time elapsed since the clock was constructed or last restarted.
		Time GetElapsed() const;
		/// @brief Reset the clock to now and return the elapsed time prior to the reset.
		Time Restart();
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
	};

	/// @brief Convenience alias: std::chrono::seconds.
	typedef std::chrono::seconds seconds;
	/// @brief Convenience alias: std::chrono::milliseconds.
	typedef std::chrono::milliseconds ms;
	/// @brief Convenience alias: std::chrono::microseconds.
	typedef std::chrono::microseconds us;

}