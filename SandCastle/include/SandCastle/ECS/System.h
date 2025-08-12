#pragma once
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Time.h"
#include <SDL3/SDL.h>
#include "SandCastle/ECS/World.h"

namespace SandCastle
{
	class Entity;

	class System
	{
	public:

		typedef enum
		{
			Update = 1,
			FixedUpdate = 2,
			Event = 4,
			ImGui = 8,
			Render = 16
		}Method;

		virtual ~System() {}

		/// @brief Called when the system is pushed within Systems
		virtual void OnStart() {}

		/// @brief Called as often as possible.
		/// @param deltaTime Time elapsed since last call.
		virtual void OnUpdate(Time deltaTime) {}

		/// @brief Called on a fixed timestep.
		virtual void OnFixedUpdate(Time fixedDeltaTime) {}

		/// @brief Called after every other OnUpdate/OnFixedUpdate
		virtual void OnLateUpdate() {}

		/// @brief This is where you can create ImGui elements
		/// Warning! Contrarlily to all the other virtual method (including OnLateUpdate)
		/// This is called from the rendering thread.
		/// 
		virtual void OnImGui() {}

		/// @brief Called every time an SDL_Event is received, if no other system has processed the event.
		/// You can use ImGui::GetIO.WantToCaptureMouse/Keyboard to know if ImGui used this event or not.
		/// @param event SDL_Event
		/// @return true if the event has been
		virtual bool OnEvent(SDL_Event& event) { return false; }

		/// @brief Called when the system is removed from Systems
		virtual void OnRemove() {}

		/// @brief Use this method in the system's constructor to override default value.
		/// @param priority If set to 0, Systems will attribute an automatic value based on the order it has been pushed.
		void SetPriority(int priority)
		{
			m_priority = priority;
		}

		/// @brief bitmask telling SandCastle wich method are being used.
		/// overriding this will help the engine save some amount of CPU power.
		virtual int GetUsedMethod() { return Update | FixedUpdate | Event | ImGui | Render; }

		/// @brief Higher priority will have it's methods called before lower priority
		virtual int GetPriority() { return m_priority; }

		/// @brief Gives an string identifier to the system, for debugging purposes
		virtual std::string DebugName() { return "System"; }

	protected:
		std::vector<World*>& GetWorlds();

		/// @brief Set a callback for every time a component of type ComponentType is created on an entity of any world
		/// Use this method in system's constructor for it to work even for component created before Engine::Launch
		/// @param callback Method to call upon creation
		/// @param priority if multiple callback are set, higher priority get called first
		template <typename ComponentType, typename SystemType>
		void ListenAddComponent(void(SystemType::* callback)(ComponentSignal), SignalPriority priority = SignalPriority::medium)
		{
			for (auto& world : GetWorlds())
			{
				world->ListenAddComponentSignal<ComponentType>(callback, static_cast<SystemType*>(this));
			}
		}

		/// @brief Set a callback for every time a component of type ComponentType is created on an entity of a specific world
		/// Use this method in system's constructor for it to work even for component created before Engine::Launch
		/// @param callback Method to call upon creation
		/// @param priority if multiple callback are set, higher priority get called first
		template <typename ComponentType, typename SystemType>
		void ListenAddComponent(World* world, void(SystemType::* callback)(ComponentSignal), SignalPriority priority = SignalPriority::medium)
		{
			world->ListenAddComponentSignal<ComponentType>(callback, static_cast<SystemType*>(this));
		}

		/// @brief Set a callback for every time a component of type ComponentType is removed on an entity of any world
		/// Use this method in system's constructor for it to work even for component created before Engine::Launch
		/// @param callback Method to call upon creation
		/// @param priority if multiple callback are set, higher priority get called first
		template <typename ComponentType, typename SystemType>
		void ListenRemoveComponent(void(SystemType::* callback)(ComponentSignal), SignalPriority priority = SignalPriority::medium)
		{
			for (auto& world : GetWorlds())
			{
				world->ListenRemoveComponentSignal<ComponentType>(callback, static_cast<SystemType*>(this));
			}
		}

		/// @brief Set a callback for every time a component of type ComponentType is removed on an entity of a specific world
		/// Use this method in system's constructor for it to work even for component created before Engine::Launch
		/// @param callback Method to call upon creation
		/// @param priority if multiple callback are set, higher priority get called first
		template <typename ComponentType, typename SystemType>
		void ListenRemoveComponent(World* world, void(SystemType::* callback)(ComponentSignal), SignalPriority priority = SignalPriority::medium)
		{
			world->ListenRemoveComponentSignal<ComponentType>(callback, static_cast<SystemType*>(this));
		}

		/// @brief Remove every callbacks for when a component of type ComponentType is added in any world
		/// Use this method in system's constructor for it to work even for component created before Engine::Launch
		/// @param callback Method to call upon creation
		/// @param priority if multiple callback are set, higher priority get called first
		template <typename ComponentType>
		void StopListenAddComponent()
		{
			for (auto& world : GetWorlds())
			{
				world->StopListenAddComponentSignal<ComponentType>(this);
			}
		}

		/// @brief Remove every callbacks for when a component of type ComponentType is added in a specific world
		/// @param callback Method to call upon creation
		/// @param priority if multiple callback are set, higher priority get called first
		template <typename ComponentType>
		void StopListenAddComponent(World* world)
		{
			world->StopListenAddComponentSignal<ComponentType>(this);
		}

		/// @brief Remove every callbacks for when a component of type ComponentType is removed in any worlds
		/// @param callback Method to call upon creation
		/// @param priority if multiple callback are set, higher priority get called first
		template <typename ComponentType>
		void StopListenRemoveComponent()
		{
			for (auto& world : GetWorlds())
			{
				world->StopListenRemoveComponentSignal<ComponentType>(this);
			}
		}

		/// @brief Remove every callbacks for when a component of type ComponentType is removed in a specific world
		/// @param callback Method to call upon creation
		/// @param priority if multiple callback are set, higher priority get called first
		template <typename ComponentType>
		void StopListenRemoveComponent(World* world)
		{
			world->ListenRemoveComponentSignal<ComponentType>(this);
		}
	
		/// @brief Invoke a lambda or a free function for each entities of every worlds containing the given components.
		/// The functor parameters will have acess to a reference of the given components.
		/// @tparam ...ComponentType Given component
		/// @param function Lambda or free function
		template <typename... ComponentType, typename Functor>
		void ForeachComponents(Functor function)
		{
			for (auto& world : GetWorlds())
			{
				auto view = world->registry.view<ComponentType...>();
				for (auto entityId : view)
				{
					[&] <std::size_t... I>(std::index_sequence<I...>)
					{
						function(std::get<I>(view.get(entityId))...);
					}(std::make_index_sequence<sizeof...(ComponentType)>());
				}
			}
		};

		/// @brief Invoke a lambda or a free function for each entities of a specific world containing the given components.
		/// The functor parameters will have acess to a reference of the given components.
		/// @tparam ...ComponentType Given component
		/// @param function Lambda or free function
		template <typename... ComponentType, typename Functor>
		void ForeachComponents(World* world, Functor function)
		{
			auto view = world->registry.view<ComponentType...>();
			for (auto entityId : view)
			{
				[&] <std::size_t... I>(std::index_sequence<I...>)
				{
					function(std::get<I>(view.get(entityId))...);
				}(std::make_index_sequence<sizeof...(ComponentType)>());
			}
		};

		/// @brief Invoke a lambda or a free function for each entities of every worlds containing the given components.
		/// The functor parameters will have acess to the entity and a reference of the given components.
		/// @tparam ...ComponentType Given component
		/// @param function Lambda or free function
		template <typename... ComponentType, typename Functor>
		void ForeachEntities(Functor function)
		{
			for (auto& world : GetWorlds())
			{
				auto view = world->registry.view<ComponentType...>();
				for (auto entityId : view)
				{
					[&] <std::size_t... I>(std::index_sequence<I...>)
					{
						Entity entity(entityId, world);
						function(entity, std::get<I>(view.get(entityId))...);
					}(std::make_index_sequence<sizeof...(ComponentType)>());
				}
			}
		};

		/// @brief Invoke a lambda or a free function for each entities of a specific world containing the given components.
		/// The functor parameters will have acess to the entity and a reference of the given components.
		/// @tparam ...ComponentType Given component
		/// @param function Lambda or free function
		template <typename... ComponentType, typename Functor>
		void ForeachEntities(World* world, Functor function)
		{
			auto view = world->registry.view<ComponentType...>();
			for (auto entityId : view)
			{
				[&] <std::size_t... I>(std::index_sequence<I...>)
				{
					Entity entity(entityId, world);
					function(entity, std::get<I>(view.get(entityId))...);
				}(std::make_index_sequence<sizeof...(ComponentType)>());
			}
		};


		/// @brief Invoke a member method of this system for each entities of every worlds containing the given components.
		/// The functor parameters will have acess to the entity and a reference of the given components.
		/// @tparam ...ComponentType Given component
		/// @param function The member function pointer
		template <typename... ComponentType, typename SystemType>
		void ForeachEntities(void(SystemType::* function)(Entity, ComponentType&...))
		{
			for (auto& world : GetWorlds())
			{
				auto view = world->registry.view<ComponentType...>();
				for (auto entityId : view)
				{
					[&] <std::size_t... I>(std::index_sequence<I...>)
					{
						Entity entity(entityId, world);
						(static_cast<SystemType*>(this)->*function)(entity, std::get<I>(view.get(entityId))...);
					}(std::make_index_sequence<sizeof...(ComponentType)>());
				}
			}
		};

		/// @brief Invoke a member method of this system for each entities of a specific world containing the given components.
		/// The functor parameters will have acess to the entity and a reference of the given components.
		/// @tparam ...ComponentType Given component
		/// @param function The member function pointer
		template <typename... ComponentType, typename SystemType>
		void ForeachEntities(World* world, void(SystemType::* function)(Entity, ComponentType&...))
		{
			auto view = world->registry.view<ComponentType...>();
			for (auto entityId : view)
			{
				[&] <std::size_t... I>(std::index_sequence<I...>)
				{
					Entity entity(entityId, world);
					(static_cast<SystemType*>(this)->*function)(entity, std::get<I>(view.get(entityId))...);
				}(std::make_index_sequence<sizeof...(ComponentType)>());
			}
		};

	private:

		int m_priority = 0;

	};
}