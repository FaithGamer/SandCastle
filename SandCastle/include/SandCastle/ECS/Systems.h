#pragma once

#include <set>

#include "SandCastle/ECS/System.h"
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Core/Container.h"
#include "SandCastle/Core/TypeId.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/Signal.h"

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

	class Systems : public Singleton<Systems>
	{

		friend sptr<Systems> Singleton<Systems>::Instance();
		friend void Singleton<Systems>::Kill();

	public:

		~Systems();

		/// @brief Add a system to the game logic.
		/// If the system priority is 0 (default), the system's push order will
		/// be their order in the queue for calling OnUpdate/OnFixedUpdate/OnRender/OnEvent
		/// with their priority set to 1+(Nb of System pushed with a priority of 0)
		/// @tparam SystemType 
		/// @tparam ...Args 
		/// @param ...args 
		template <typename SystemType, typename... Args>
		static void Push(Args&&... args)
		{
			System* system = new SystemType(std::forward<Args>(args)...);
			if (system->GetPriority() == 0)
			{
				//Priority based on push order
				system->SetPriority(-(++Systems::Instance()->m_pushCount));
			}
			Systems::Instance()->m_pendingSystemIn.push_back(SystemIdPriority(system, TypeId::GetId<SystemType>(), system->GetPriority()));
		}
		template <typename SystemType>
		static void Push()
		{
			System* system = new SystemType;
			if (system->GetPriority() == 0)
			{
				//Priority based on push order
				system->SetPriority(-(++Systems::Instance()->m_pushCount));
			}
			Systems::Instance()->m_pendingSystemIn.push_back(SystemIdPriority(system, TypeId::GetId<SystemType>(), system->GetPriority()));
		}
		template <typename SystemType>
		static SystemType* Get()
		{
			auto ins = Instance();
			int32_t typeId = TypeId::GetId<SystemType>();
			auto system = ins->m_allSystems.find(typeId);
			if (system == ins->m_allSystems.end())
				return nullptr;
			return static_cast<SystemType*>(system->second);
		}
		template <typename SystemType>
		static void Remove()
		{
			Systems::Instance()->m_pendingSystemOut.push_back(TypeId::GetId<SystemType>());
		}
		template <typename SystemType>
		static bool HasSystem()
		{
			return Systems::Instance()->HasSystem(TypeId::GetId<SystemType>());
		}

		/// @brief Create and store a World, usually you will have only one World
		/// Default World name will be World_0, 1, 2, 3....
		/// @return The World created
		static World* CreateWorld();
		/// @brief Create and store a World, usually you will have only one World
		/// @param name, Give the game world a name.
		/// @return The World created
		static World* CreateWorld(std::string name);
		static void DestroyWorld(std::string name);

		/// @brief Set the camera used for rendering into the window by the SandCastle rendering Systems (like the SpriteRenderSystem)
		/// Note that the entities rendered by the SandCastle Systems will be those within the same world as the camera.
		/// @param camera Pointer to the camera, the Camera component is a PointableComponenent
		static void SetMainCamera(Camera* camera);

		/// @brief Label a world as being main. This will be the world you get from GetMainWorld.
		/// @param world 
		static void SetMainWorld(World* world);

		/// @brief Set the interval between two FixedUpdate call
		/// @param seconds Interval in seconds
		static void SetFixedUpdateTime(float seconds);

		/// @brief Set a multiplier to the deltaTime and fixedDeltaTime
		static void SetTimeScale(float scale);

		/// @brief The world wich is set as main, by default it is the first created world
		/// @return World pointer.
		static World* GetMainWorld();

		/// @brief Get a World from it's name.
		/// @param World's name 
		/// @return The game world, nullptr if doesn't exists.
		static World* GetWorld(std::string name);

		/// @brief Access all the game worlds
		/// @return Reference to the Worlds vector.
		static std::vector<World*>& GetWorlds();

		/// @brief Return camera used for window rendering
		/// @return main camera
		static Camera* GetMainCamera();

		/// @brief Return the mouse position in wolrd unit relative to the main camera
		/// @return mouse position
		static Vec2f GetMouseWorldPos();
	public:
		SignalSender<int> lateRenderSignal;
	private:

		struct Worlds
		{
			void Push(World* world);
			void Destroy(std::string name);
			World* Get(std::string name);
			std::vector<World*> pointers;
			std::vector<std::string> names;
			World* mainWorld = nullptr;
		};
		friend Engine;
		Systems();

		void Update();
		void HandleWindowEvents(SDL_Event& event);
		void IntegratePending();
		void RemovePending();
		void RemovePending(std::vector<SystemIdPriority>& systems, int32_t system, std::set<SystemIdPriority, CompareSystemId>& toDelete);
		bool HasSystem(int32_t typeId);

		Clock m_updateClock;
		Clock m_fixedUpdateClock;
		Time m_fixedUpdateAccumulator;
		int m_maxFixedUpdate;

		Worlds m_worlds;
		std::unordered_map<int32_t, System*> m_allSystems;

		std::vector<SystemIdPriority> m_eventSystems;
		std::vector<SystemIdPriority> m_fixedUpdateSystems;
		std::vector<SystemIdPriority> m_updateSystems;
		std::vector<SystemIdPriority> m_imGuiSystems;
		std::vector<SystemIdPriority> m_renderSystems;

		Camera* m_mainCamera = nullptr;

		std::vector<SystemIdPriority> m_pendingSystemIn;
		std::vector<int32_t> m_pendingSystemOut;

		int m_pushCount;
	};
}