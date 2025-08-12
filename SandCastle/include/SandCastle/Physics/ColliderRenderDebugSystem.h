#pragma once

#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	class ColliderRenderDebugSystem : public System
	{
	public:
		ColliderRenderDebugSystem();
		void OnStart() override;
		void OnUpdate(Time delta) override;
		void OnLateUpdate() override;
		int GetUsedMethod() override;
		void OnRemove() override;

		/// @brief Construct the collider render entities that have been queued up.
		///By default this method is called automatically OnUpdate
		///You can disable this behaviour with UpdateQueueAuto
		void UpdateQueue();

		/// @brief Set false if you want to control when the collider render entities are being constructed with UpdateQueue.
		/// Typical usage: you're iterating entities/components on differents thread and you want to visualize colliders.
		/// Set to true by default.
		void UpdateQueueAuto(bool updateQueuAuto);

		void OnAddKinematicBody(ComponentSignal signal);
		void OnAddStaticBody(ComponentSignal signal);
		void OnRemoveKinematicBody(ComponentSignal signal);
		void OnRemoveStaticBody(ComponentSignal signal);
	private:
		bool m_updateQueueAuto;
		uint32_t m_debugLayer;
		std::unordered_set<EntityId> m_newStaticBodies;
		std::unordered_set<EntityId> m_newKinematicBodies;

		std::unordered_set<EntityId> m_removedStaticBodies;
		std::unordered_set<EntityId> m_removedKinematicBodies;
	};
}

