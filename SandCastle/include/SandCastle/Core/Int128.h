#pragma once
#include <boost/int128.hpp>

namespace SandCastle
{
	/// @brief 128-bit signed integer (alias for boost::int128_t). Used where 64 bits is too small (idle/incremental counters, big economies).
	typedef boost::int128::int128_t Int128;
}

#include "SandCastle/Core/Math.h"
namespace SandCastle
{
	namespace Math
	{
		inline std::string FormatCompact(Int128 value)
		{
			return FormatCompact(static_cast<double>(value), 0);
		}
	}
}
