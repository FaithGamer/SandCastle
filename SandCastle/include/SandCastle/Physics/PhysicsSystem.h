#pragma once
#include "SandCastle/ECS/System.h"
//must associate every new body with their entity through user data

namespace SandCastle
{
	/// @brief Engine system that steps the Box2D world and syncs Body positions
	/// back to entity Transforms each frame. Push it via Systems::Push<PhysicsSystem>().
	class PhysicsSystem : public System
	{
	public:
		PhysicsSystem();
		void Start() override;
		void Update() override;
		int GetUsedMethod() override;
		//void OnAddKinematicBody(ComponentSignal signal);
	//	void OnAddStaticBody(ComponentSignal signal);

	};
}