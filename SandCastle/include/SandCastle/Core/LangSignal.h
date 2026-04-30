#pragma once
#include "SandCastle/Core/std_macros.h"

/// @brief Payload broadcast by Assets when the active language changes.
/// Localized text/font/UI listeners react to this to swap their content.
struct LangSignal
{
	String lang;
};
