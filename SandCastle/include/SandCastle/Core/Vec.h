#pragma once
#include <glm/glm.hpp>
#include <math.h>
#include <box2d/b2_math.h>

namespace SandCastle
{

	typedef glm::vec4 Vec4f;
	typedef glm::mat4 Mat4;
	typedef glm::mat3 Mat3;

	//////////
	//
	//Vector3
	//
	//////////

	template <class T>
	class Vec3
	{
	public:

		T x;
		T y;
		T z;

		constexpr Vec3(T X, T Y, T Z) : x(X), y(Y), z(Z)
		{

		}
		Vec3() : x(0), y(0), z(0)
		{

		}
		constexpr Vec3(b2Vec2 vec) : x((T)vec.x), y((T)vec.y), z((T)0)
		{

		}
		constexpr Vec3(glm::vec3 glm) : x((T)glm.x), y((T)glm.y), z((T)glm.z)
		{

		}
		constexpr Vec3(T value) : x(value), y(value), z(value)
		{

		}

		template <class T>
		Vec3(Vec3<T> v) : x(v.x), y(v.y), z(v.z)
		{

		}

		//Math

		/// @brief Pythagorus
		inline float Magnitude() const
		{
			return sqrt(x * x + y * y + z * z);
		}
		/// @brief Magnitude of the difference with another vector
		inline float Distance(Vec3<T> vec) const
		{
			return Vec3<T>(x - vec.x, y - vec.y, z - vec.z).Magnitude();
		}
		/// @brief Return the normalized vector without modifying it.
		inline Vec3<T> Normalized() const
		{
			float mag = Magnitude();
			if (mag == 0)
				return Vec3<T>(x, y, z);
			return Vec3<T>(x / mag, y / mag, z / mag);
		}
		/// @brief Reflected vector off the plane defined by a normal
		inline Vec3<T> Reflected(Vec3<T> normal) const
		{
			float dot = Dot(normal);

			return Vec3<T>(
				x - 2 * dot * normal.x,
				y - 2 * dot * normal.y,
				z - 2 * dot * normal.z);
		}
		/// @brief Dot product
		inline float Dot(const Vec3<T> v) const
		{
			return x * v.x + y * v.y + z * v.z;
		}
		inline Vec3<T> Cross(const Vec3<T> v)
		{
			return Vec3<T>(
				(y * v.z) - (z * v.y),
				(z * v.x) - (x * v.z),
				(x * v.y) - (y * v.x));
		}
		/// @brief Moved vector towards target without over
		inline Vec3<T> MovedTowards(Vec3<T> target, float maxDistance) const
		{
			Vec3<T> diff = Vec3<T>(target.x - x, target.y - y, target.z - z);
			diff = diff.ClampedMagnitude(maxDistance);
			return *this + diff;
		}
		/// @brief Angle between two vectors, Ignore the Z axis, considering the Vec3 as Vec2.
		/// @return Angle in degrees
		inline float Angle(const Vec3<T> v) const
		{
			float dot = x * v.x + y * v.y;
			float det = x * v.y - y * v.y;

			return glm::degrees(atan2(det, dot));
		}
		/// @brief Return a copy of vector with its magnitude clamped to maxLength
		inline Vec3<T> ClampedMagnitude(float maxLength) const
		{
			float mag = Magnitude();
			mag = mag > maxLength ? maxLength : mag;
			return Normalized() * mag;
		}

		/// @brief Perform a normalization on the vector
		inline void Normalize()
		{
			float mag = Magnitude();
			if (mag == 0)
				return;
			x = x / mag;
			y = y / mag;
			z = z / mag;
		}
		//Utils
		inline std::string ToString() const
		{
			std::string X = std::to_string(x);
			std::string Y = std::to_string(y);
			std::string Z = std::to_string(z);
			return "[" + X + "], [" + Y + "], " + "[" + Z + "]";
		}
		inline operator glm::vec3() const
		{
			return glm::vec3{ x, y, z };
		}
		inline operator b2Vec2() const
		{
			return b2Vec2(x, y);
		}
		inline Vec3<T> operator-()
		{
			return Vec3<float>(-x, -y, -z);
		}
		inline Vec3<float> operator+=(Vec3<float> rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}
		inline Vec3<float> operator-=(Vec3<float> rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}
		/*inline operator std::tuple<float, float, float>()
		{
			return std::make_tuple(x, y, z);
		}*/
	};

	///////////
	//
	//Vector2
	//
	///////////

	template <class T>
	class Vec2
	{
	public:

		T x;
		T y;

		constexpr Vec2(T X, T Y) : x(X), y(Y)
		{

		}
		Vec2() : x(0), y(0)
		{

		}
		constexpr Vec2(glm::vec2 glm) : x((T)glm.x), y((T)glm.y)
		{

		}
		constexpr Vec2(b2Vec2 vec) : x((T)vec.x), y((T)vec.y)
		{

		}
		constexpr Vec2(T value) : x((T)value), y((T)value)
		{

		}
		template <class T>
		constexpr Vec2(Vec2<T> v) : x((T)v.x), y((T)v.y)
		{

		}
		template <class T>
		constexpr Vec2(Vec3<T> v3) : x((T)v3.x), y((T)v3.y)
		{

		}

		//Math

		/// @brief Pythagorus
		inline float Magnitude() const
		{
			return sqrt(x * x + y * y);
		}
		/// @brief Magnitude of the difference with another vector
		inline float Distance(Vec2<T> vec) const
		{
			return Vec2<T>(x - vec.x, y - vec.y).Magnitude();
		}
		/// @brief Return the normalized vector without modifying it.
		inline Vec2<T> Normalized() const
		{
			float mag = Magnitude();
			if (mag == 0)
				return Vec2<T>(x, y);
			return Vec2<T>(x / mag, y / mag);
		}
		/// @brief Reflected vector off the plane defined by a normal
		inline Vec2<T> Reflected(Vec2<T> normal) const
		{
			float dot = Dot(normal);

			return Vec2<T>(
				x - 2 * dot * normal.x,
				y - 2 * dot * normal.y);
		}
		/// @brief Dot product
		inline float Dot(const Vec2<T> v) const
		{
			return x * v.x + y * v.y;
		}
		/// @brief Moved vector towards target without over
		inline Vec2<T> MovedTowards(Vec2<T> target, float maxDistance) const
		{
			Vec2<T> diff = Vec2<T>(target.x - x, target.y - y);
			diff = diff.ClampedMagnitude(maxDistance);
			return *this + diff;
		}
		/// @brief Angle between two vectors
		/// @return Angle in degrees
		inline float Angle(const Vec2<T> v) const
		{
			float dot = x * v.x + y * v.y;
			float det = x * v.y - y * v.y;

			return glm::degrees(atan2(det, dot));
		}
		/// @brief Return a copy of vector with its magnitude clamped to maxLength
		inline Vec2<T> ClampedMagnitude(float maxLength) const
		{
			float mag = Magnitude();
			mag = mag > maxLength ? maxLength : mag;
			return Normalized() * mag;
		}

		/// @brief Perform a normalization on the vector
		inline void Normalize()
		{
			float mag = Magnitude();
			if (mag == 0)
				return;
			x = x / mag;
			y = y / mag;
		}
		//Utils
		inline std::string ToString() const
		{
			std::string X = std::to_string(x);
			std::string Y = std::to_string(y);
			return "[" + X + "], [" + Y + "]";
		}
		inline operator glm::vec2() const
		{
			return glm::vec2{ (float)x, (float)y };
		}
		inline operator Vec3<T>() const
		{
			return Vec3<T>(x, y, 0);
		}
		inline operator b2Vec2() const
		{
			return b2Vec2((float)x, (float)y);
		}
		inline Vec2<T> operator-()
		{
			return Vec2<float>(-x, -y);
		}
		inline Vec2<float> operator+=(Vec2<float> rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}
		inline Vec2<float> operator-=(Vec2<float> rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}
	};

	//Operators

	//Vector3

	inline Vec3<float> operator+(Vec3<float> lhs, Vec3<float> rhs)
	{
		return Vec3<float>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
	}
	inline Vec3<float> operator-(Vec3<float> lhs, Vec3<float> rhs)
	{
		return Vec3<float>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
	}
	inline Vec3<float> operator*(Vec3<float> vec3, const float f)
	{
		return Vec3<float>(vec3.x * f, vec3.y * f, vec3.z * f);
	}
	inline Vec3<float> operator*(const float f, Vec3<float> vec3)
	{
		return Vec3<float>(vec3.x * f, vec3.y * f, vec3.z * f);
	}
	inline Vec3<float> operator*(Vec3<float> lhs, Vec3<float> rhs)
	{
		return Vec3<float>(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
	}
	inline Vec3<float> operator*=(Vec3<float>& vec3, const float f)
	{
		vec3 = Vec3<float>(vec3.x * f, vec3.y * f, vec3.z * f);
		return vec3;
	}
	inline Vec3<float> operator*=(Vec3<float>& lhs, Vec3<float> rhs)
	{
		lhs = Vec3<float>(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
		return lhs;
	}
	inline Vec3<float> operator/(Vec3<float> vec3, const float f)
	{
		return Vec3<float>(vec3.x / f, vec3.y / f, vec3.z / f);
	}
	inline Vec3<float> operator/=(Vec3<float>& vec3, const float f)
	{
		vec3 = Vec3<float>(vec3.x / f, vec3.y / f, vec3.z / f);
		return vec3;
	}
	inline bool operator==(Vec3<float> lhs, Vec3<float> rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}

	//Vector2

	inline Vec2<float> operator+(Vec2<float> lhs, Vec2<float> rhs)
	{
		return Vec2<float>(lhs.x + rhs.x, lhs.y + rhs.y);
	}
	inline Vec2<float> operator-(Vec2<float> lhs, Vec2<float> rhs)
	{
		return Vec2<float>(lhs.x - rhs.x, lhs.y - rhs.y);
	}
	inline Vec2<float> operator*(Vec2<float> vec3, const float f)
	{
		return Vec2<float>(vec3.x * f, vec3.y * f);
	}
	inline Vec2<float> operator*(const float f, Vec2<float> vec3)
	{
		return Vec2<float>(vec3.x * f, vec3.y * f);
	}
	inline Vec2<float> operator*(Vec2<float> lhs, Vec2<float> rhs)
	{
		return Vec2<float>(lhs.x * rhs.x, lhs.y * rhs.y);
	}
	inline Vec2<float> operator*=(Vec2<float>& vec2, const float f)
	{
		vec2 = Vec2<float>(vec2.x * f, vec2.y * f);
		return vec2;
	}
	inline Vec2<float> operator*=(Vec2<float>& lhs, Vec2<float> rhs)
	{
		lhs = Vec2<float>(lhs.x * rhs.x, lhs.y * rhs.y);
		return lhs;
	}
	inline Vec2<float> operator/(Vec2<float> vec2, const float f)
	{
		return Vec2<float>(vec2.x / f, vec2.y / f);
	}
	inline Vec2<float> operator/=(Vec2<float>& vec2, const float f)
	{
		vec2 = Vec2<float>(vec2.x / f, vec2.y / f);
		return vec2;
	}
	inline bool operator==(Vec2<float> lhs, Vec2<float> rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}

	inline Vec4f operator-(Vec4f lhs, Vec2<float> rhs)
	{
		return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z, lhs.w };
	}

	//Static vector methods
	class Vec
	{
	public:
		template <class T>
		inline static float Distance(Vec3<T> lhs, Vec3<T> rhs)
		{
			return Vec3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z).Magnitude();
		}
		template <class T>
		inline static float Distance(Vec2<T> lhs, Vec2<T> rhs)
		{
			return Vec2<T>(lhs.x - rhs.x, lhs.y - rhs.y).Magnitude();
		}
		template <class T>
		inline static Vec3<T> Lerp(Vec3<T> min, Vec3<T> max, float t)
		{

			return Vec3<T>(
				min.x * (1 - t) + max.x * t,
				min.y * (1 - t) + max.y * t,
				min.z * (1 - t) + max.z * t);
		}
	};

	typedef Vec3<float> Vec3f;
	typedef Vec3<int> Vec3i;
	typedef Vec3<unsigned int> Vec3u;

	typedef Vec2<float> Vec2f;
	typedef Vec2<int> Vec2i;
	typedef Vec2<unsigned int> Vec2u;

}