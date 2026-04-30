#pragma once

/// @file ECS.h
/// @brief Aggregate include for the ECS module: the EnTT-backed Entity/Component
/// store and the engine's System scheduler. Game logic is organized as
/// subclasses of System pushed via Systems::Push<T>(); state lives on Entities
/// added via Entity::Create() and decorated with components via Entity::Add<T>().

#include "SandCastle/ECS/Systems.h"
#include "SandCastle/ECS/System.h"
#include "SandCastle/ECS/Entity.h"