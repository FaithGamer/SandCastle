#pragma once
#include <math.h>
#ifndef PI
#define PI 3.141592653589793
#endif
namespace SandCastle
{
	/// @brief Standard easing curves f(t): [0,1] -> [0,1] used to shape animation
	/// timings, particle interpolation, UI transitions, etc.
	/// "In" eases at the start, "Out" eases at the end, "InOut" eases both.
	namespace Easing
	{
		/// @brief Sine-shaped ease-in then ease-out. t in [0,1].
		double SineInOut(double t);
		/// @brief Sine-shaped ease-in (slow start, linear-ish end).
		double SineIn(double t);
		/// @brief Sine-shaped ease-out (linear-ish start, slow end).
		double SineOut(double t);
		/// @brief Quadratic ease-in then ease-out.
		double QuadInOut(double t);
		/// @brief Quadratic ease-in (t*t).
		double QuadIn(double t);
		/// @brief Quadratic ease-out.
		double QuadOut(double t);
		/// @brief Cubic ease-in then ease-out.
		double CubicInOut(double t);
		/// @brief Cubic ease-in (t*t*t).
		double CubicIn(double t);
		/// @brief Cubic ease-out.
		double CubicOut(double t);
		/// @brief Circular ease-in then ease-out (curve based on a quarter circle).
		double CircInOut(double t);
		/// @brief Circular ease-in.
		double CircIn(double t);
		/// @brief Circular ease-out.
		double CircOut(double t);
		/// @brief Exponential ease-in then ease-out (very sharp ramp).
		double ExpoInOut(double t);
		/// @brief Exponential ease-in (2^(10t-10)).
		double ExpoIn(double t);
		/// @brief Exponential ease-out.
		double ExpoOut(double t);
		/// @brief Decaying-oscillation ease-out (overshoots target then settles).
		double ElasticOut(double t);
	}
}