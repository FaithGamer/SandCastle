#pragma once
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Time.h"
#include <SDL3/SDL.h>

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

		/// @brief Invoke a lambda or a free function for each entities of every worlds containing the given components.
		/// The functor parameters will have acess to a reference of the given components.
		/// @tparam ...ComponentType Given component
		/// @param function Lambda or free function
		template <typename... ComponentType, typename Functor>
		void ForeachComponents(Functor function)
		{

			auto view = Entity::registry.view<ComponentType...>();
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
			auto view = Entity::registry.view<ComponentType...>();
			for (auto entityId : view)
			{
				[&] <std::size_t... I>(std::index_sequence<I...>)
				{
					Entity entity(entityId);
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
			auto view = Entity::registry.view<ComponentType...>();
			for (auto entityId : view)
			{
				[&] <std::size_t... I>(std::index_sequence<I...>)
				{
					Entity entity(entityId);
					(static_cast<SystemType*>(this)->*function)(entity, std::get<I>(view.get(entityId))...);
				}(std::make_index_sequence<sizeof...(ComponentType)>());
			}
		};
	private:

		int m_priority = 0;

	};
}