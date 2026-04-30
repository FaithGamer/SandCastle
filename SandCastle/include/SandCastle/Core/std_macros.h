#pragma once
#include <memory>
#include <string>
/// @file std_macros.h
/// @brief Short aliases for common standard-library types used throughout the engine API.
#define makesptr std::make_shared
#define sptr std::shared_ptr
#define uptr std::unique_ptr
#define makeuptr std::make_unique
#define wptr std::weak_ptr
#define MakePair std::make_pair

/// @brief Engine-wide alias for std::string.
typedef std::string String;
