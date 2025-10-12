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
			Updt = 1,
			FixedUpdt = 2,
			Event = 4,
			ImGui = 8,
			LateUpdt = 16
		}Method;

		virtual ~System() {}

		/// @brief Called once when before update
		virtual void Start() {}
		/// @brief Called once when before update and after start
		virtual void PostStart() {}

		/// @brief Called as often as possible.
		/// @param deltaTime Time elapsed since last call.
		virtual void Update() {}

		/// @brief Called on a fixed timestep.
		virtual void FixedUpdate() {}

		/// @brief Called after every other Update/OnFixedUpdate
		virtual void LateUpdate() {}

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
		virtual int GetUsedMethod() { return Updt | FixedUpdt | Event | ImGui | LateUpdt; }

		/// @brief Higher priority will have it's methods called before lower priority
		virtual int GetPriority() { return m_priority; }

		/// @brief Gives an string identifier to the system, for debugging purposes
		virtual std::string DebugName() { return "System"; }

	private:

		int m_priority = 0;

	};
}