#pragma once

#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	/// @brief Debug system that draws collider wireframes for every Body in the world.
	/// Push it via Systems::Push<ColliderRenderDebugSystem>() during development.
	class ColliderRenderDebugSystem : public System
	{
	public:
		ColliderRenderDebugSystem();
		void Start() override;
		void Update() override;
		/*void LateUpdate() override;
		int GetUsedMethod() override;
		void OnRemove() override;*/

		/// @brief Construct the collider render entities that have been queued up.
		///By default this method is called automatically Update
		///You can disable this behaviour with UpdateQueueAuto
		void UpdateQueue();

		/// @brief Set false if you want to control when the collider render entities are being constructed with UpdateQueue.
		/// Typical usage: you're iterating entities/components on differents thread and you want to visualize colliders.
		/// Set to true by default.
		void UpdateQueueAuto(bool updateQueuAuto);

		/*void OnAddKinematicBody(ComponentSignal signal);
		void OnAddStaticBody(ComponentSignal signal);
		void OnRemoveKinematicBody(ComponentSignal signal);
		void OnRemoveStaticBody(ComponentSignal signal);*/
	private:
		bool m_updateQueueAuto;
		uint32_t m_debugLayer;
		/*std::unordered_set<EntityId> m_newStaticBodies;
		std::unordered_set<EntityId> m_newKinematicBodies;

		std::unordered_set<EntityId> m_removedStaticBodies;
		std::unordered_set<EntityId> m_removedKinematicBodies;*/
	};
}

