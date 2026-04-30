#pragma once

/// @file Fancy.h
/// @brief Optional shorthand macros enabled by defining SC_FANCY.
/// Provides terse aliases (e.g. `sys(T)` for `Systems::Get<T>()`).

#ifdef SC_FANCY
/// @brief Shorthand for Systems::Get<T>(). Available only when SC_FANCY is defined.
#define sys(system) Systems::Get<system>()
#endif