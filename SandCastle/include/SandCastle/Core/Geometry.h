#pragma once
#include <math.h>
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	namespace Geometry
	{
		/// @brief Calculate intersection point between two line (even if line are too short)
		/// @param l1s first list start point
		/// @param l1e fisrt line end point
		/// @param l2s second line start point
		/// @param l2e second line end point
		/// @return Point of intersection, may return 0, 0 if vector are parrallel while logging a warning.
		inline Vec2f LinesIntersection(Vec2f l1s, Vec2f l1e, Vec2f l2s, Vec2f l2e)
		{
			//thanks chat gpt
			Vec2f l1d = (l1e - l1s).Normalized();
			Vec2f l2d = (l2e - l2s).Normalized();

			// Calculate determinant
			double det = l1d.x * l2d.y - l2d.x * l1d.y;

			// If determinant is zero, lines are parallel
			if (det == 0) {
				LOG_WARN("No intersection between parallel lines");
				return { 0, 0 }; // Indicate no intersection
			}

			// Calculate parameter for each line
			double t1 = (l2d.y * (l2s.x - l1s.x) + l2d.x * (l1s.y - l2s.y)) / det;
			double t2 = (l1d.x * (l2s.y - l1s.y) + l1d.y * (l1s.x - l2s.x)) / det;

			// Calculate intersection point
			return Vec2f(l1s.x + (float)t1 * l1d.x, l1s.y + (float)t1 * l1d.y);
		}
	}
}