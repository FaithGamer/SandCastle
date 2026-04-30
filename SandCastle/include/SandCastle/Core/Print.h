#pragma once

#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	/// @brief Debug pretty-printers for engine math types (logged via the engine logger).
	namespace Print
	{
		/// @brief Log a 4x4 matrix as a readable grid for debugging.
		void mat4(Mat4 mat);
	}
}
