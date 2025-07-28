#pragma once
#include "SandCastle/Core/Log.h"
#include <cassert>
#define ENTT_ASSERT(condition, msg) \
if(!(condition)) \
{ \
	SandCastle::Log::Instance()->GetLogger()->error("Entt assertion failed: {0}", (msg)); \
	SandCastle::Log::Instance()->GetLogger()->flush(); \
	assert(condition); \
}
#include <entt/entt.hpp>