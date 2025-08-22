#pragma once
#include "SandCastle/Core/std_macros.h"
#include "EntityId.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	class Transform;

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

		/// @brief An invalid entity handle, use it to store an entity later
		Entity() = default;
		/// @brief Create a handle to an existing entity of the main world
		Entity(EntityId entityId);
		/// @brief Create an entity in the main world
		static Entity Create();

		/// @brief Destroy all entities having the mentioned components
		template <typename... Component>
		static void DestroyAll()
		{
			registry.clear<Component...>();
		}
		template<typename Type, typename... Other, typename... Exclude>
		inline static auto
			View(entt::exclude_t<Exclude...> exclude = entt::exclude_t{})
		{
			return registry.view<Type, Other..., Exclude...>(exclude);
		}
		template<typename Type, typename... Other, typename... Exclude>
		inline static auto
			Group(entt::exclude_t<Exclude...> exclude = entt::exclude_t{})
		{
			return registry.group<Type, Other..., Exclude...>(exclude);
		}

		void AddChild(Entity entity);
		void RemoveChild(EntityId entity);
		void Unparent();

		/// @brief Check validity of entity.
		/// @return true if valid
		bool Valid();

		/// @brief Add a component if it doesn't exists yet.
		/// @param args Parameters for the component constructor.
		/// @return Added component, or the one already in place.
		template <typename Component, typename... Args>
		Component* AddComponent(Args&&... args)
		{
			if (!Valid())
			{
				LOG_WARN("Trying to add a component to an invalid entity!");
				return nullptr;
			}
			return &registry.get_or_emplace<Component>(m_id, std::forward<Args>(args)...);
		}
		/// @brief Remove a component
		/// @tparam Component 
		template <typename Component>
		void RemoveComponent()
		{
			registry.remove<Component>(m_id);
		}
		/// @brief Access an entity component if it exists.
		/// Do not store the pointer as it may be invalidated.
		/// @return Component pointer, nullptr if doesn't exists.
		template <typename Component>
		Component* GetComponent()
		{
			return registry.try_get<Component>(m_id);
		}
		
		/// @brief Access an entity component if it exists
		/// @return Component reference, nullptr if doesn't exists.
		template <typename Component>
		Component* GetComponentNoCheck()
		{
			return &registry.get<Component>(m_id);
		}
		/// @brief Get the EntityId
		/// The EntityId will remain the same during the entity lifetime.
		/// It can be used to retreive the entity from it's World with no overhead.
		/// @return The EntityId
		EntityId GetId() const
		{
			return m_id;
		}
		template <typename T>
		constexpr T* gc()
		{
			return GetComponent<T>();
		}
		Transform* gtr();
		template <typename T>
		constexpr T* adc()
		{
			return AddComponent<T>();
		}

		/// @brief Destroy the entity and it's components, and does the same for every children
		/// Trying to access or add components after using this method
		/// will result in undefined behaviour
		void Destroy();

		inline bool operator==(const Entity& rhs) const
		{
			return m_id == rhs.m_id;
		}

		static entt::registry registry;
	private:
		void DestroyFromParent();
		/// @brief Called by remove child
		void JustUnparent();
		/// @brief Called by remove parent
		void JustRemoveChild(EntityId id);
		Entity(EntityId, entt::registry* registry);
		EntityId m_id = EntityId(0);
		bool m_valid = false;

	public:

		/// @brief Create an entity with Transform and SpriteRender component
		///			at position 0, 0, 0
		/// @return The entity created
		static Entity CreateSprite(String defaultSprite = "square.png_0_0");
		/// @brief Create an entity with Transform and SpriteRender 
		/// and Animator component at position 0, 0, 0
		/// @param defaultAnimaion the animation that will be played by default
		/// @return 
		static Entity CreateAnimatedSprite(String defaultAnimation = "anim_test.anim",
			String defaultAnimStateName = "default");

	};
}