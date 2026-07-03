#include "pch.h"
#include "SandCastle/Core/Easing.h"
namespace SandCastle
{

	double Easing::SineInOut(double t)
	{
		return 0.5 * (1 + sin(3.1415926 * (t - 0.5)));

	}
	double Easing::SineIn(double t)
	{
		return sin(1.5707963 * t);
	}
	double Easing::SineOut(double t)
	{
		return 1 + sin(1.5707963 * (--t));
	}
	double Easing::QuadInOut(double t)
	{
		return t < 0.5 ? 2 * t * t : t * (4 - 2 * t) - 1;
	}
	double Easing::QuadIn(double t)
	{
		return t * t;
	}
	double Easing::QuadOut(double t)
	{
		return t * (2 - t);
	}
	double Easing::CubicInOut(double t)
	{
		// 4t^3 below the midpoint, 1 + 4(t-1)^3 above. (The old one-liner decremented
		// t twice via two unsequenced (--t), returning values far outside [0,1] for
		// t just past 0.5 — anything lerped with it overshot wildly at mid-curve.)
		if (t < 0.5)
			return 4 * t * t * t;
		const double f = t - 1;
		return 1 + 4 * f * f * f;
	}
	double Easing::CubicIn(double t)
	{
		return t * t * t;
	}
	double Easing::CubicOut(double t)
	{
		// 1 + (t-1)^3, without the unsequenced (--t) of the old one-liner.
		const double f = t - 1;
		return 1 + f * f * f;
	}
	double Easing::CircInOut(double t)
	{
		if (t < 0.5) {
			return (1 - sqrt(1 - 2 * t)) * 0.5;
		}
		else {
			return (1 + sqrt(2 * t - 1)) * 0.5;
		}
	}
	double Easing::CircIn(double t)
	{
		return 1 - sqrt(1 - t);
	}
	double Easing::CircOut(double t)
	{
		return sqrt(t);
	}
	double Easing::ExpoInOut(double t) {
		if (t < 0.5) {
			return (pow(2, 16 * t) - 1) / 510;
		}
		else {
			return 1 - 0.5 * pow(2, -16 * (t - 0.5));
		}
	}
	double Easing::ExpoIn(double t) {
		return (pow(2, 8 * t) - 1) / 255;
	}

	double Easing::ExpoOut(double t) {
		return 1 - pow(2, -8 * t);
	}

	double Easing::ElasticOut(double t) {
		if (t == 0) return 0;
		if (t == 1) return 1;

		const double c4 = (2 * PI) / 3;
		return pow(2, -10 * t) * sin((t * 10 - 0.75) * c4) + 1;
	}


	
}