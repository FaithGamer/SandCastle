#pragma once

#include "SandCastle/Entt.h"

/// @brief Stable identifier for an entity in the global EnTT registry. Cheap to copy/store; remains valid for the lifetime of the entity.
typedef entt::entity EntityId;
