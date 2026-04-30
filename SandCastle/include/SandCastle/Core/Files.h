#pragma once

namespace SandCastle
{
	/// @brief File-system helpers.
	namespace Files
	{
		/// @brief Read an open ifstream to its end and return the contents as a string.
		std::string IfstreamToString(std::ifstream& file);
	};
}