#pragma once
/// @file Entt.h
/// @brief Engine-side wrapper that pulls in EnTT and routes its assertions
/// through the engine's logger before triggering the platform assert.
#include "SandCastle/Core/Log.h"
#include <cassert>
/// @brief EnTT assertion hook: logs the failing condition through spdlog before tripping the C assert. Forwards `condition` and `msg` from the EnTT internals.
#define ENTT_ASSERT(condition, msg) \
if(!(condition)) \
{ \
	SandCastle::Log::Instance()->GetLogger()->error("Entt assertion failed: {0}", (msg)); \
	SandCastle::Log::Instance()->GetLogger()->flush(); \
	assert(condition); \
}
#include <entt/entt.hpp>