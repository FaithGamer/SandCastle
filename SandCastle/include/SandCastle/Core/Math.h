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

		/// @brief Format a number into a compact, human-readable string.
		///
		/// Sub-1000 values:
		///   Trailing zeros are stripped up to maxFloatPrecision decimal places.
		///   3.5000 -> "3.5"  |  3.54 -> "3.54"  |  3.0 -> "3"
		///   3.54 (maxFloatPrecision=1) -> "3.5"
		///
		/// Scaled values (>= 1000):
		///   1000    -> "1.00k"  |  10000  -> "10.0k"  |  100000 -> "100k"
		///   1.5e9   -> "1.50B"  |  1e15   -> "1.0Q"  |  1e18   -> "1.0e18"
		///
		/// Suffixes: k (thousand), M (million), B (billion), T (trillion), Q (quadrillion).
		/// Above 999Q uses scientific notation ("N.DeNN" or "N.De-N", always 6 chars).
		/// Positive exponents omit the + sign; negative exponents include the - sign.
		///
		/// @param value             The value to format. Negative values are treated as positive.
		/// @param maxFloatPrecision Max decimal places shown for sub-1000 values (default 2).
		inline std::string FormatCompact(double value, int maxFloatPrecision = 2)
		{
			// Sanitise input
			if (value < 0.0) value = -value;

			char buf[16];
			int  len = 0;

			// ---- Numbers below 1000: format with trimmed decimals ----
			if (value < 1000.0)
			{
				// Clamp precision to a sane range
				if (maxFloatPrecision < 0) maxFloatPrecision = 0;
				if (maxFloatPrecision > 9) maxFloatPrecision = 9;

				// Multiply by 10^maxFloatPrecision, round, then work in integer space
				// e.g. value=3.54, precision=2  -> scaled=354, divisor=100
				//      value=3.5,  precision=2  -> scaled=350, divisor=100
				static constexpr uint64_t pow10[10] = {
					1, 10, 100, 1000, 10000, 100000, 1000000,
					10000000, 100000000, 1000000000
				};
				uint64_t divisor = pow10[maxFloatPrecision];
				uint64_t scaled = (uint64_t)(value * (double)divisor + 0.5);

				// Guard: rounding pushed us to 1000 (e.g. 999.9995 with precision 3)
				if (scaled >= 1000ULL * divisor)
					scaled = 1000ULL * divisor - 1;

				uint64_t intPart = scaled / divisor;
				uint64_t fracPart = scaled % divisor;  // still has leading zeros baked in

				// Write integer part (1–3 digits)
				if (intPart >= 100) {
					buf[len++] = (char)('0' + intPart / 100);
					buf[len++] = (char)('0' + (intPart / 10) % 10);
					buf[len++] = (char)('0' + intPart % 10);
				}
				else if (intPart >= 10) {
					buf[len++] = (char)('0' + intPart / 10);
					buf[len++] = (char)('0' + intPart % 10);
				}
				else { buf[len++] = (char)('0' + intPart); }

				// Write fractional part, stripping trailing zeros
				if (maxFloatPrecision > 0 && fracPart > 0)
				{
					// Emit digits of fracPart left-to-right, then trim trailing zeros.
					// We reconstruct them into a small temp array to do the trim.
					char frac[10];
					int  flen = 0;
					uint64_t remaining = fracPart;
					// fracPart is already normalised to [0, divisor), but leading zeros
					// in the fractional portion must be preserved (e.g. 3.05 -> fracPart=5
					// with divisor=100, which should print ".05" not ".5").
					// Emit all maxFloatPrecision digits, then strip trailing zeros.
					for (int d = maxFloatPrecision - 1; d >= 0; --d)
					{
						uint64_t place = pow10[d];
						frac[flen++] = (char)('0' + (char)(remaining / place));
						remaining %= place;
					}
					// Strip trailing zeros
					while (flen > 0 && frac[flen - 1] == '0') --flen;

					if (flen > 0)
					{
						buf[len++] = '.';
						for (int i = 0; i < flen; ++i) buf[len++] = frac[i];
					}
				}

				return { buf, (size_t)len };
			}

			// ---- Scale detection: find the largest suffix where value >= scale ----
			static constexpr double scales[5] = { 1e3, 1e6, 1e9, 1e12, 1e15 };
			static constexpr char   suffixes[5] = { 'k', 'M', 'B', 'T', 'Q' };

			int idx = 4;
			while (idx > 0 && value < scales[idx]) --idx;

			double s = value / scales[idx];

			// ---- Values >= 1e18: scientific notation "N.DeNN" or "N.De-N" (always 6 chars) ----
			// Positive exponents omit the + sign (user knows from context), negative exponents keep the -
			if (idx == 4 && s >= 1000.0)
			{
				// Precomputed powers of 10 lookup table (initialized once, eliminates std::pow call)
				static double pow10_cache[309] = {};
				static bool initialized = false;
				if (!initialized)
				{
					pow10_cache[0] = 1.0;
					for (int i = 1; i < 309; ++i)
						pow10_cache[i] = pow10_cache[i - 1] * 10.0;
					initialized = true;
				}

				int exp = (int)std::floor(std::log10(value));
				if (exp < 0) exp = 0;      // clamp to valid range
				if (exp >= 309) exp = 308; // clamp to valid range

				double mantissa = value / pow10_cache[exp];
				int leading = (int)mantissa;
				int decimal = (int)((mantissa - leading) * 10.0 + 0.5);  // first decimal digit
				if (decimal >= 10) { decimal = 9; }  // clamp rounding

				buf[0] = (char)('0' + leading);
				buf[1] = '.';
				buf[2] = (char)('0' + decimal);
				buf[3] = 'e';

				if (exp < 0)
				{
					// Negative exponent: "N.De-D"
					buf[4] = '-';
					exp = -exp;
					if (exp > 9) exp = 9;  // clamp single-digit negative
					buf[5] = (char)('0' + exp);
					return { buf, 6 };
				}
				else
				{
					// Positive exponent: "N.DeNN" (no + sign)
					if (exp > 99) exp = 99;  // clamp to 2 digits
					buf[4] = (char)('0' + exp / 10);
					buf[5] = (char)('0' + exp % 10);
					return { buf, 6 };
				}
			}

			// ---- Format scaled value using only integer arithmetic ----
			// Clamp after rounding to stay within the chosen tier. Values at the
			// boundary (e.g. 9999 -> s=9.999, n=1000) are kept in their current
			// tier rather than promoted, so the displayed suffix always matches
			// the true magnitude.

			if (s < 10.0)
			{
				int n = (int)(s * 100.0 + 0.5);
				if (n > 999) n = 999;
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
		inline std::string FormatCompact(int64_t value) { return FormatCompact((double)value, 0); }
		inline std::string FormatCompact(int     value) { return FormatCompact((double)value, 0); }

		inline String ToString(double value, int precision = 0)
		{
			std::ostringstream out;
			out << std::fixed << std::setprecision(precision) << value;
			return out.str();
		}

		inline String ToString(float value, int precision = 0)
		{
			std::ostringstream out;
			out << std::fixed << std::setprecision(precision) << value;
			return out.str();
		}

	}
}

#include "Math.tpp"