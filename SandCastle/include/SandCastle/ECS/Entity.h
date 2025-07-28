#pragma once
#include "SandCastle/Core/std_macros.h"
#include "EntityId.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	class World;

	/// @brief For internal use, makes the entity a parent
	struct Children
	{
		std::unordered_set<EntityId> children;
	};

	/// @brief For internal use, makes the entity a child
	struct Parent
	{
		EntityId parent;
	};

	class Entity
	{
	public:

		/// @brief An invalid entity handle, use it to store an entity later and check validity
		Entity() = default;
		/// @brief Create a handle to an existing entity of the main world
		Entity(EntityId entityId);
		/// @brief Create a handle to an existing entity of a specific world
		Entity(EntityId entityId, World* world);
		/// @brief Create an entity in the main world
		static Entity Create();
		/// @brief Create an entity in a specified world
		static Entity Create(World* world);

		void AddChild(Entity entity);
		void RemoveChild(EntityId entity);
		void Unparent();

		

		bool Valid();

		/// @brief Add a component if it doesn't exists yet.
		/// @param args Parameters for the component constructor.
		/// @return Added component, or the one already in place.
		template <typename ComponentType, typename... Args>
		ComponentType* AddComponent(Args&&... args)
		{
			if (!Valid())
			{
				LOG_WARN("Trying to add a component to an invalid entity!");
				return nullptr;
			}
			return &m_registry->get_or_emplace<ComponentType>(m_id, std::forward<Args>(args)...);
		}
		/// @brief Remove a component
		/// @tparam ComponentType 
		template <typename ComponentType>
		void RemoveComponent()
		{
			m_registry->remove<ComponentType>(m_id);
		}
		/// @brief Access an entity component if it exists.
		/// Do not store the pointer as it may be invalidated.
		/// @return Component pointer, nullptr if doesn't exists.
		template <typename ComponentType>
		ComponentType* GetComponent()
		{
			return m_registry->try_get<ComponentType>(m_id);
		}
		/// @brief Access an entity component if it exists
		/// @return Component reference, nullptr if doesn't exists.
		template <typename ComponentType>
		ComponentType* GetComponentNoCheck()
		{
			return &m_registry->get<ComponentType>(m_id);
		}
		/// @brief Get the EntityId
		/// The EntityId will remain the same during the entity lifetime.
		/// It can be used to retreive the entity from it's World with no overhead.
		/// @return The EntityId
		inline EntityId GetId() const
		{
			return m_id;
		}
		/// @brief Destroy the entity and it's components, and does the same for every children
		/// Trying to access or add components after using this method
		/// will result in undefined behaviour
		void Destroy();

		bool operator==(const Entity& rhs) const
		{
			return m_id == rhs.m_id && m_registry == rhs.m_registry;
		}

	private:
		void DestroyFromParent();
		/// @brief Called by remove child
		void JustUnparent();
		/// @brief Called by remove parent
		void JustRemoveChild(EntityId id);
		Entity(EntityId, entt::registry* registry);
		EntityId m_id = EntityId(0);
		entt::registry* m_registry = nullptr;
		bool m_valid = false;
	};
}