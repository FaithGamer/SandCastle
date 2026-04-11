#pragma once
#include "SandCastle/Core/std_macros.h"
#include "EntityId.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	class Transform;
	class SpriteRender;
	class Sprite;
	struct Animation;

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
		static size_t Count();
		template<typename ...Component>
		inline size_t CountOf()
		{
			return registry.storage<Component...>().free_list();
		}
		/// @brief Destroy all entities having the mentioned components
		template <typename... Component>
		static void DestroyAll()
		{
			auto view = View<Component...>();
			view.each([&](auto entity, auto&...)
				{
					Entity(entity).Destroy();
				});
		}

		/// @brief Create a view over entities having Component
		/// use when you iterate one component
		template<typename Component, typename... Other, typename... Exclude>
		inline static auto
			View(entt::exclude_t<Exclude...> exclude = entt::exclude_t{})
		{
			return registry.view<Component, Other..., Exclude...>(exclude);
		}

		/// @brief The child's transform become affected by the parent's transform
		/// @param entity 
		void AddChild(Entity entity);
		std::optional<std::unordered_set<EntityId>::iterator> RemoveChild(EntityId entity);
		void RemoveAllChildren();
		void Unparent();

		/// @brief Check validity of entity.
		/// @return true if valid
		bool Valid();

		/// @brief Add a component if it doesn't exists yet.
		/// @param args Parameters for the component constructor.
		/// @return Added component, or the one already in place.
		template<typename Component, typename... Args>
		Component* AddGet(Args&&... args)
		{
			static_assert(!std::is_same_v<std::remove_cvref_t<Component>, Transform>,
				"Use Add() for Transform.  Don't get until SpriteRender has been added");
			static_assert(!std::is_same_v<std::remove_cvref_t<Component>, SpriteRender>,
				"Use Add() for SpriteRender. Don't get until Transform has been added");

			if (!Valid()) {
				LOG_WARN("Trying to add a component to an invalid entity!");
				return nullptr;
			}
			return &registry.get_or_emplace<Component>(m_id, std::forward<Args>(args)...);
		}
		template<typename Component, typename... Args>
		void Add(Args&&... args)
		{
			if (!Valid()) {
				LOG_WARN("Trying to add a component to an invalid entity!");
				return;
			}
			registry.get_or_emplace<Component>(m_id, std::forward<Args>(args)...);
		}
		/// @brief Remove a component
		/// @tparam Component 
		template <typename Component>
		void Remove()
		{
			registry.remove<Component>(m_id);
		}
		/// @brief Access an entity component if it exists.
		/// Do not store the pointer as it may be invalidated.
		/// @return Component pointer, nullptr if doesn't exists.
		template <typename Component>
		Component* Get()
		{
			return registry.try_get<Component>(m_id);
		}

		/// @brief Access an entity component if it exists
		/// @return Component reference, nullptr if doesn't exists.
		template <typename Component>
		Component* GetNoCheck()
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
		Transform* gtr();

		/// @brief Destroy the entity and it's components, and does the same for every children
		/// Trying to access or add components after using this method
		/// will result in undefined behaviour
		void Destroy();

		/// @brief Get the first component in the registry
		/// Using this for components that appear only once in the registry.
		template <class T>
		static T* GetFirst()
		{
			return Entity(Entity::View<T>().front()).Get<T>();
		}

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
		static Entity CreateSprite(Sprite* sprite);
		/// @brief Create an entity with Transform and SpriteRender 
		/// and Animator component at position 0, 0, 0
		/// @param defaultAnimaion the animation that will be played by default
		/// @return 
		static Entity CreateAnimatedSprite(String defaultAnimation = "anim_test.anim",
			String defaultAnimStateName = "default");
		static Entity CreateAnimatedSprite(String animId, Animation* anim);

	};
}