#pragma once
#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	/// @brief Engine system, pushed by default by Engine::Init.
	/// Each fixed timestep: steps the Box2D world and snapshots every DynamicBody's
	/// simulated state. Each frame: syncs every KinematicBody from the Transform on
	/// the same entity, and writes every DynamicBody's state to its Transform,
	/// interpolated between the last two fixed steps (smooth visuals at any
	/// framerate, up to one fixed step of latency).
	class PhysicsSystem : public System
	{
	public:
		PhysicsSystem();
		void Start() override;
		void Update() override;
		void FixedUpdate() override;
		int GetUsedMethod() override;

	};
}
