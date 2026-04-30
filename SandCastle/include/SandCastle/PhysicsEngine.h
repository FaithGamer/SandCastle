#pragma once

/// @file PhysicsEngine.h
/// @brief Aggregate include for the Physics module: a Box2D wrapper exposing
/// Body components (Static/Kinematic/Dynamic), Colliders (Box2D, Circle2D,
/// Polygon2D), AABB/raycast/overlap queries, and the PhysicsSystem that steps
/// the simulation each frame.

#include "SandCastle/Physics/AABBQueries.h"
#include "SandCastle/Core/Bitmask.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Physics/Physics.h"
#include "SandCastle/Physics/PhysicsSystem.h"
