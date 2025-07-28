#pragma once
#include <math.h>
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	namespace Math
	{
		inline int Abs(int value)
		{
			return std::abs(value);
		}
		inline float Abs(float value)
		{
			return std::abs(value);
		}
		inline float Degrees(float radians)
		{
			return glm::degrees(radians);
		}
		inline float Radians(float degrees)
		{
			return glm::radians(degrees);
		}
		///@brief Scale a value ranged in between two number, to another range in between two other numbers.
		template <typename T>
		T ScaleRangeTo(T value, T oldMin, T oldMax, T newMin, T newMax)
		{
			T oldRange = (oldMax - oldMin);
			T newRange = (newMax - newMin);
			return (((value - oldMin) * newRange) / oldRange) + newMin;
		}

		/// @brief Return direction of degrees in vector
		inline Vec2f AngleToVec(float degrees)
		{
			return { cos(glm::radians(degrees)), sin(glm::radians(degrees)) };
		}

		/// @brief Return direction of vector in degrees
		inline float VecToAngle(Vec2f vector)
		{
			return glm::degrees(atan2(vector.y, vector.x));
		}

		/// @brief Calculate intersection point between two line (even if line are too short)
		/// @param l1s first list start point
		/// @param l1e fisrt line end point
		/// @param l2s second line start point
		/// @param l2e second line end point
		/// @return Point of intersection, may return 0, 0 if vector are parrallel while logging a warning.
		inline Vec2f LineIntersection2D(Vec2f l1s, Vec2f l1e, Vec2f l2s, Vec2f l2e)
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
		template <typename T>
		inline constexpr T Min(T lhs, T rhs)
		{
			return std::min(lhs, rhs);
		}

		template <typename T>
		inline constexpr T Max(T lhs, T rhs)
		{
			return std::max(lhs, rhs);
		}

		inline float Sin(float degrees)
		{
			return std::sin(glm::radians(degrees));
		}

		inline float Cos(float degrees)
		{
			return std::cos(glm::radians(degrees));
		}

		inline float Lerp(float min, float max, float t)
		{
			return min * (1 - t) + max * t;
		}

		template <typename T>
		inline Vec2<T> Lerp(const Vec2<T> min, const Vec2<T> max, float t)
		{
			return Vec2<T>(
				min.x * (1 - t) + max.x * t,
				min.y * (1 - t) + max.y * t);
		}

		template <typename T>
		inline Vec3<T> Lerp(const Vec3<T> min, const Vec3<T> max, float t)
		{
			return Vec3<T>(
				min.x * (1 - t) + max.x * t,
				min.y * (1 - t) + max.y * t,
				min.z * (1 - t) + max.z * t);
		}

		inline float Clamp(float value, float min, float max)
		{
			const float t = value < min ? min : value;
			return t > max ? max : t;
		}
		inline float Clamp01(float value)
		{
			const float t = value < 0 ? 0 : value;
			return t > 1 ? 1 : t;
		}
		///@brief Loops the value t, so that it is never larger than length and never smaller than 0.
		inline float Repeat(float t, float length)
		{
			return Clamp(t - std::floor(t / length) * length, 0.0f, length);
		}
		template <typename T>
		inline int Sign(T val)
		{
			return (T(0) < val) - (val < T(0));
		}
		///@brief Calculates the shortest difference between two given angles.
		inline float DeltaAngle(float current, float target)
		{
			float delta = Repeat((target - current), 360.0F);
			if (delta > 180.0F)
				delta -= 360.0F;
			return delta;
		}
		inline float MoveTowards(float current, float target, float maxDelta)
		{
			if (abs(target - current) <= maxDelta)
				return target;
			return current + Sign(target - current) * maxDelta;
		}
		inline float MoveTowardsAngle(float current, float target, float maxDelta)
		{
			float deltaAngle = DeltaAngle(current, target);
			if (-maxDelta < deltaAngle && deltaAngle < maxDelta)
				return target;
			target = current + deltaAngle;
			return MoveTowards(current, target, maxDelta);
		}
	}
}

#include "Math.tpp"