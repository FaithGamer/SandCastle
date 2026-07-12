#pragma once
#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	/// @brief Engine system that syncs every KinematicBody component with the
	/// Transform component on the same entity, each frame. The world is used for
	/// collision queries only and is not stepped. Pushed by default by Engine::Init.
	class PhysicsSystem : public System
	{
	public:
		PhysicsSystem();
		void Start() override;
		void Update() override;
		int GetUsedMethod() override;

	};
}
