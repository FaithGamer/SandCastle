#pragma once

#include <set>

#include "SandCastle/ECS/System.h"
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Core/Container.h"
#include "SandCastle/Core/TypeId.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/Signal.h"
#include "SandCastle/Render/Camera.h"

namespace SandCastle
{
	class World;
	class Camera;

	struct SystemIdPriority
	{
		System* system;
		int32_t typeId;
		int priority;

		bool operator==(const SystemIdPriority& other)
		{
			return typeId == other.typeId;
		}
		bool operator==(const int32_t other)
		{
			return typeId == other;
		}
	};

	struct CompareSystemPriority
	{
		bool operator()(const SystemIdPriority& l, const SystemIdPriority& r) const
		{
			return l.priority > r.priority;
		}
	};

	struct CompareSystemId
	{
		bool operator()(const SystemIdPriority& l, const SystemIdPriority& r) const
		{
			return l.typeId < r.typeId;
		}
	};

	/// @brief Game-loop scheduler holding every active System.
	/// Call Push<T>() to add a system, Get<T>() to retrieve it, Remove<T>() to
	/// remove it. The engine drives Start/Update/FixedUpdate/LateUpdate/OnEvent/
	/// OnImGui at the right times based on each system's GetUsedMethod() bitmask.
	class Systems : public Singleton<Systems>
	{

		friend sptr<Systems> Singleton<Systems>::Instance();
		friend void Singleton<Systems>::Kill();

	public:

		~Systems();

		/// @brief Add a system to the game logic.
		/// If the system priority is 0 (default), the system's push order will
		/// be their order in the queue for calling Update/OnFixedUpdate/OnLateUpdate/OnEvent
		/// with their priority set to 1+(Nb of System pushed with a priority of 0)
		/// @tparam SystemType 
		/// @tparam ...Args 
		/// @param ...args 
		template <typename SystemType, typename... Args>
		static SystemType* Push(Args&&... args)
		{
			SystemType* system = new SystemType(std::forward<Args>(args)...);
			if (system->GetPriority() == 0)
			{
				//Priority based on push order
				system->SetPriority(-(++Systems::Instance()->m_pushCount));
			}
			Systems::Instance()->m_pendingSystemIn.push_back(SystemIdPriority(system, TypeId::GetId<SystemType>(), system->GetPriority()));
			return system;
		}
		/// @brief Default-construct a System and queue it for insertion. The actual insertion happens at the next safe point in the loop.
		template <typename SystemType>
		static SystemType* Push()
		{
			SystemType* system = new SystemType;
			if (system->GetPriority() == 0)
			{
				//Priority based on push order
				system->SetPriority(-(++Systems::Instance()->m_pushCount));
			}
			Systems::Instance()->m_pendingSystemIn.push_back(SystemIdPriority(system, TypeId::GetId<SystemType>(), system->GetPriority()));
			return system;
		}
		/// @brief Retrieve a previously pushed system by type. Errors if the system is not present.
		template <typename SystemType>
		static SystemType* Get()
		{
			auto ins = Instance();
			int32_t typeId = TypeId::GetId<SystemType>();
			auto system = ins->m_allSystems.find(typeId);
			ASSERT_LOG_ERROR((system != ins->m_allSystems.end()), "System doesn't exists!");
			return static_cast<SystemType*>(system->second);
		}
		/// @brief Queue a system for removal at the next safe point in the loop.
		template <typename SystemType>
		static void Remove()
		{
			Systems::Instance()->m_pendingSystemOut.push_back(TypeId::GetId<SystemType>());
		}
		/// @brief True if the scheduler currently holds a system of this type.
		template <typename SystemType>
		static bool HasSystem()
		{
			return Systems::Instance()->HasSystem(TypeId::GetId<SystemType>());
		}

		/// @brief For internal use only.
		void ImGuiUpdates();

		/// @brief Set the interval between two FixedUpdate call
		/// @param seconds Interval in seconds
		static void SetFixedUpdateTime(float seconds);

		/// @brief Set a multiplier to the deltaTime and fixedDeltaTime
		static void SetTimeScale(float scale);
	private:

		friend Engine;
		Systems();
		void Init();

		void Update();
		void WindowEvents(SDL_Event& event);
		void IntegratePending();
		void RemovePending();
		void RemovePending(std::vector<SystemIdPriority>& systems, int32_t system, std::set<SystemIdPriority, CompareSystemId>& toDelete);
		bool HasSystem(int32_t typeId);

		Clock m_updateClock;
		Clock m_fixedUpdateClock;
		Time m_fixedUpdateAccumulator;
		int m_maxFixedUpdate;

		std::unordered_map<int32_t, System*> m_allSystems;

		std::vector<SystemIdPriority> m_eventSystems;
		std::vector<SystemIdPriority> m_fixedUpdateSystems;
		std::vector<SystemIdPriority> m_updateSystems;
		std::vector<SystemIdPriority> m_imGuiSystems;
		std::vector<SystemIdPriority> m_lateUpdateSystems;

		Camera m_defaultCamera;

		std::vector<SystemIdPriority> m_pendingSystemIn;
		std::vector<int32_t> m_pendingSystemOut;

		int m_pushCount;
	};
}