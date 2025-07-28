#pragma once
#include <math.h>
#ifndef PI
#define PI 3.141592653589793
#endif
namespace SandCastle
{
	namespace Easing
	{
		double SineInOut(double t);
		double SineIn(double t);
		double SineOut(double t);
		double QuadInOut(double t);
		double QuadIn(double t);
		double QuadOut(double t);
		double CubicInOut(double t);
		double CubicIn(double t);
		double CubicOut(double t);
		double CircInOut(double t);
		double CircIn(double t);
		double CircOut(double t);
		double ExpoInOut(double t);
		double ExpoIn(double t);
		double ExpoOut(double t);
		double ElasticOut(double t);
	}
}