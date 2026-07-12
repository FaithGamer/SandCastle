#pragma once

/// @file PhysicsEngine.h
/// @brief Aggregate include for the Physics module: a Box2D (v3) wrapper exposing
/// Body components (Static/Kinematic), Colliders (Box2D, Circle2D,
/// Polygon2D), raycast/overlap queries, and the PhysicsSystem that syncs
/// kinematic bodies with their entity's Transform.

#include "SandCastle/Core/Bitmask.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Physics/Physics.h"
#include "SandCastle/Physics/PhysicsSystem.h"
