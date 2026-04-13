#pragma once
#include <math.h>
#include <string>
#include <cstdint>
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
			degrees += 90;
			return { cos(glm::radians(degrees)), sin(glm::radians(degrees)) };
		}

		/// @brief Return direction of vector in degrees
		inline float VecToAngle(Vec2f vector)
		{
			return glm::degrees(atan2(vector.y, vector.x)) - 90;
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
		template <typename T>
		inline T NearestMultiple(T input, T step)
		{
			if (input < step)
				return step;
			if (step == 0)
				return input;
			T quotient = static_cast<T>(std::round(static_cast<double>(input) / step));
			return quotient * step;
		}
		template <typename T>
		inline T FloorMultiple(T input, T step)
		{
			if (input < step)
				return step;
			if (step == 0) 
				return input;
			T quotient = input / step;
			return quotient * step;
		}
		template <typename T>
		inline T CeilMultiple(T input, T step)
		{
			if (input < step)
				return step;
			if (step == 0)
				return input;
			T quotient = static_cast<T>(std::ceil(static_cast<double>(input) / step));
			return quotient * step;
		}
		/// @brief Round to the nearest power of two.
		/// @param value value to be rounded
		/// @param min minimum returned value
		/// @return the rounded value
		inline float RoundPow2(float value, float min = 0.125f) 
		{
			if (value <= 0.0f) 
				return min;

			// compute log base 2
			float exponent = std::log2(value);

			// round exponent to nearest integer
			int nearestExp = static_cast<int>(std::round(exponent));

			// compute 2^nearestExp
			float result = std::pow(2.0f, nearestExp);
			if (result < min) 
				result = min;

			return result;
		}
		/// @brief Floor to the nearest power of two.
		/// @param value value to be floored
		/// @param min minimum returned value
		/// @return the rounded value
		inline float FloorPow2(float value, float min = 0.125f)
		{
			if (value <= 0.0f)
				return min;

			// compute log base 2
			float exponent = std::log2(value);

			// round exponent to nearest integer
			int nearestExp = static_cast<int>(std::floor(exponent));

			// compute 2^nearestExp
			float result = std::pow(2.0f, nearestExp);
			if (result < min)
				result = min;

			return result;
		}
		/// @brief Ceil to the nearest power of two.
		/// @param value value to be ceiled
		/// @param min minimum returned value
		/// @return the rounded value
		inline float CeilPow2(float value, float min = 0.125f)
		{
			if (value <= 0.0f)
				return min;

			// compute log base 2
			float exponent = std::log2(value);

			// round exponent to nearest integer
			int nearestExp = static_cast<int>(std::ceil(exponent));

			// compute 2^nearestExp
			float result = std::pow(2.0f, nearestExp);
			if (result < min)
				result = min;

			return result;
		}
		template <typename T>
		T FloorToEven(T value)
		{
			if (value % 2 > 0)
				return value - 1;
			return value;
		}

		/// @brief Format a number into a compact, human-readable string of at most 5 characters.
		/// Uses no locale, no heap allocation, and no standard format calls — pure integer arithmetic on a stack buffer.
		/// Examples: 1000 -> "1.00k" | 10000 -> "10.0k" | 100000 -> "100k" | 1.5e9 -> "1.50B" | 1e15 -> "1e+15"
		/// Suffixes: k (thousand), M (million), B (billion), T (trillion). Above 1T uses scientific notation.
		inline std::string FormatCompact(double value)
		{
			char buf[8];
			int  len = 0;

			// ---- Numbers below 1000: write as plain integer ----
			if (value < 1000.0)
			{
				uint32_t n = (uint32_t)value;
				if      (n >= 100) { buf[len++] = (char)('0' + n / 100); buf[len++] = (char)('0' + (n / 10) % 10); buf[len++] = (char)('0' + n % 10); }
				else if (n >= 10)  { buf[len++] = (char)('0' + n / 10);  buf[len++] = (char)('0' + n % 10); }
				else               { buf[len++] = (char)('0' + n); }
				return { buf, (size_t)len };
			}

			// ---- Scale detection: find the largest suffix where value >= scale ----
			static constexpr double scales[4]  = { 1e3, 1e6, 1e9, 1e12 };
			static constexpr char   suffixes[4] = { 'k', 'M', 'B', 'T'  };

			int idx = 3;
			while (idx > 0 && value < scales[idx]) --idx;

			double s = value / scales[idx];

			// ---- Values >= 1e15: scientific notation "1e+NN" (always exactly 5 chars) ----
			if (idx == 3 && s >= 1000.0)
			{
				int exp = (int)std::log10(value);
				if (exp > 99) exp = 99; // clamp to 2 digits — doubles top out at ~e308 anyway
				buf[0] = '1'; buf[1] = 'e'; buf[2] = '+';
				buf[3] = (char)('0' + exp / 10);
				buf[4] = (char)('0' + exp % 10);
				return { buf, 5 };
			}

			// ---- Format scaled value using only integer arithmetic ----
			// s in [1,   10)  -> "X.XX" + suffix  (5 chars, 2 decimals)
			// s in [10, 100)  -> "XX.X" + suffix  (5 chars, 1 decimal)
			// s in [100,1000) -> "XXX"  + suffix  (4 chars, 0 decimals)
			if (s < 10.0)
			{
				int n = (int)(s * 100.0 + 0.5);
				if (n > 999) n = 999; // guard rounding at boundary (e.g. 9.995)
				buf[0] = (char)('0' + n / 100);
				buf[1] = '.';
				buf[2] = (char)('0' + (n / 10) % 10);
				buf[3] = (char)('0' + n % 10);
				buf[4] = suffixes[idx];
				return { buf, 5 };
			}

			if (s < 100.0)
			{
				int n = (int)(s * 10.0 + 0.5);
				if (n > 999) n = 999;
				buf[0] = (char)('0' + n / 100);
				buf[1] = (char)('0' + (n / 10) % 10);
				buf[2] = '.';
				buf[3] = (char)('0' + n % 10);
				buf[4] = suffixes[idx];
				return { buf, 5 };
			}

			{
				int n = (int)(s + 0.5);
				if (n > 999) n = 999;
				buf[0] = (char)('0' + n / 100);
				buf[1] = (char)('0' + (n / 10) % 10);
				buf[2] = (char)('0' + n % 10);
				buf[3] = suffixes[idx];
				return { buf, 4 };
			}
		}
		inline std::string FormatCompact(int64_t value) { return FormatCompact((double)value); }
		inline std::string FormatCompact(int     value) { return FormatCompact((double)value); }
	}
}

#include "Math.tpp"