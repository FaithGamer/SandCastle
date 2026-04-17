#pragma once
#include <boost/int128.hpp>

namespace SandCastle
{
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
