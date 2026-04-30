#pragma once

#include "SandCastle/Input/Input.h"

namespace SandCastle
{
	/// @brief Reserved Input variant for free-form text entry. Not yet wired up.
	class TextualInput
	{
	public:
		InputType GetType() const { return InputType::Textual; }

	};
}