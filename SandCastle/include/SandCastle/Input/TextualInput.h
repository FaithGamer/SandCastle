#pragma once

#include "SandCastle/Input/Input.h"

namespace SandCastle
{
	class TextualInput 
	{
	public:
		InputType GetType() const { return InputType::Textual; }

	};
}