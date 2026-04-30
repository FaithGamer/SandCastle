#pragma once
#include <string>

/// @brief Asset alias: a localized text blob loaded by the Assets store. Same storage as String, distinct type so Assets::Get<Textual>(...) resolves to the localized version.
typedef std::string Textual;
