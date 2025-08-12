#include "pch.h"

#include "imgui/imgui_impl_sdl3.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/Internal/ImGuiLoader.h"
#include "SandCastle/Engine.h"
#include "SandCastle/ECS/World.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/Camera.h"
#include "SandCastle/Input/Mouse.h"
#include "SandCastle/Core/Profiling.h"

namespace SandCastle
{
	Systems::Systems() :
		m_pushCount(0),
		m_maxFixedUpdate(3)
	{

	}

	void Systems::Init()
	{
		SetMainCamera(&m_defaultCamera);
		Window::GetResizeSignal()->Listen(&Camera::SetAspectRatio, &m_defaultCamera);
		m_defaultCamera.SetAspectRatio(Window::GetAspectRatio());
	}

	Systems::~Systems()
	{
		std::set<SystemIdPriority, CompareSystemId> toDelete;

		for (auto& system : m_eventSystems)
		{
			toDelete.insert(system);
		}
		for (auto& system : m_fixedUpdateSystems)
		{
			toDelete.insert(system);
		}
		for (auto& system : m_updateSystems)
		{
			toDelete.insert(system);
		}
		for (auto& system : m_imGuiSystems)
		{
			toDelete.insert(system);
		}
		for (auto& system : m_pendingSystemIn)
		{
			toDelete.insert(system);
		}
		for (auto& system : m_lateUpdateSystems)
		{
			toDelete.insert(system);
		}

		//Call delete only once per system

		for (auto& system : toDelete)
		{
			delete system.system;
		}


	}

	void Systems::Update()
	{
		START_PROFILING("cpu_main");
		START_PROFILING("frame_time");
		//If less than one microseconds elapse between two call
		//the m_updateClock.Restart increment doesn't accurately describe time passing by.
		Time::delta = (float)m_updateClock.Restart();
		float deltaScaled = (float)Time::delta * Time::timeScale;

		IntegratePending();
		RemovePending();

		SDL_Event event;
		while (SDL_PollEvent(&event) != 0)
		{
			HandleWindowEvents(event);
			bool imGuiEventHandled = false;
#ifdef SC_IMGUI
			ImGui_ImplSDL3_ProcessEvent(&event);
#endif
			for (auto& eventSystem : m_eventSystems)
			{
				if (eventSystem.system->OnEvent(event))
				{
					break;
				}
			}
		}

		m_fixedUpdateAccumulator += m_fixedUpdateClock.Restart();
		int i = 0;
		while (m_fixedUpdateAccumulator >= Time::fixedDelta)
		{
			Time scaledFixedDelta = (float)Time::fixedDelta * Time::timeScale;
			m_fixedUpdateAccumulator -= Time::fixedDelta;
			for (auto& system : m_fixedUpdateSystems)
			{
				system.system->OnFixedUpdate(scaledFixedDelta);
			}
			if (++i > m_maxFixedUpdate)
			{
				m_fixedUpdateAccumulator = 0;
				break; //Simulation no longer accurate
			}
		}

		for (auto& system : m_updateSystems)
		{
			system.system->OnUpdate(deltaScaled);
		}
		for (auto& system : m_lateUpdateSystems)
		{
			system.system->OnLateUpdate();
		}
		//End CPU Time
		STOP_PROFILING("cpu_main");
		Renderer2D::Instance()->Process();
		STOP_PROFILING("frame_time");
	}

	void Systems::HandleWindowEvents(SDL_Event& event)
	{
		if (event.type == SDL_EVENT_QUIT)
		{
			Engine::Stop();
			return;
		}
		switch (event.type)
		{
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			Window::Instance()->OnSDLPixelSizeChanged(event);
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			Window::Instance()->OnSDLWindowResized(event);
			break;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			Window::Instance()->FocusSignal.Send(true);
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			Window::Instance()->FocusSignal.Send(false);
			break;
		case SDL_EVENT_WINDOW_MINIMIZED:
			Window::Instance()->MinimizedSignal.Send(true);
			break;
		case SDL_EVENT_WINDOW_RESTORED:
			Window::Instance()->MinimizedSignal.Send(false);
			break;
		default:
			break;
		}
	}

	void Systems::IntegratePending()
	{
		if (m_pendingSystemIn.empty())
			return;
		std::multiset<SystemIdPriority, CompareSystemPriority> mustCallOnStart;

		for (auto& system : m_pendingSystemIn)
		{
			if (!HasSystem(system.typeId))
			{
				int usedMethodBitmask = system.system->GetUsedMethod();
				if (usedMethodBitmask & System::Method::Update)
				{
					m_updateSystems.push_back(system);
					std::sort(m_updateSystems.begin(), m_updateSystems.end(), CompareSystemPriority());
				}
				if (usedMethodBitmask & System::Method::Event)
				{
					m_eventSystems.push_back(system);
					std::sort(m_eventSystems.begin(), m_eventSystems.end(), CompareSystemPriority());
				}
				if (usedMethodBitmask & System::Method::FixedUpdate)
				{
					m_fixedUpdateSystems.push_back(system);
					std::sort(m_fixedUpdateSystems.begin(), m_fixedUpdateSystems.end(), CompareSystemPriority());
				}
				if (usedMethodBitmask & System::Method::ImGui)
				{
					m_imGuiSystems.push_back(system);
					std::sort(m_imGuiSystems.begin(), m_imGuiSystems.end(), CompareSystemPriority());
				}
				if (usedMethodBitmask & System::Method::Render)
				{
					m_lateUpdateSystems.push_back(system);
					std::sort(m_lateUpdateSystems.begin(), m_lateUpdateSystems.end(), CompareSystemPriority());
				}
				m_allSystems.insert(std::make_pair(system.typeId, system.system));
				mustCallOnStart.insert(system);
			}
			else
			{
				delete system.system;
				LOG_WARN("Trying to push system that already exists."); //To do: add debug name to systems
			}
		}

		//Call OnStart with right priority order
		for (auto& system : mustCallOnStart)
		{
			system.system->OnStart();
		}

		m_pendingSystemIn.clear();
	}

	void Systems::RemovePending(std::vector<SystemIdPriority>& systems, int32_t system, std::set<SystemIdPriority, CompareSystemId>& toDelete)
	{
		int64_t index = Container::FindIndex(systems, system);
		if (index != -1)
		{
			toDelete.insert(systems[index]);
			Container::RemoveAt(systems, index);
			std::sort(systems.begin(), systems.end(), CompareSystemPriority());
		}
	}

	void Systems::RemovePending()
	{
		if (m_pendingSystemOut.empty())
			return;
		std::set<SystemIdPriority, CompareSystemId> toDelete;

		for (auto typeId : m_pendingSystemOut)
		{
			if (HasSystem(typeId))
			{
				RemovePending(m_updateSystems, typeId, toDelete);
				RemovePending(m_fixedUpdateSystems, typeId, toDelete);
				RemovePending(m_eventSystems, typeId, toDelete);
				RemovePending(m_imGuiSystems, typeId, toDelete);
				RemovePending(m_lateUpdateSystems, typeId, toDelete);
				m_allSystems.erase(typeId);
			}
			else
			{
				LOG_WARN("Trying to remove system that doesn't exists"); //To do: add debug name to systems
			}
		}

		//To do: test to make sure it works properly
		//Call OnRemove only once and in the right order then delete.
		std::set<SystemIdPriority, CompareSystemPriority> toDeleteSorted;
		for (auto& system : toDelete)
		{
			toDeleteSorted.insert(system);
		}
		for (auto& system : toDeleteSorted)
		{
			system.system->OnRemove();
			delete system.system;
		}

		m_pendingSystemOut.clear();
	}

	bool Systems::HasSystem(int32_t typeId)
	{
		if (Container::Contains(m_updateSystems, typeId) ||
			Container::Contains(m_fixedUpdateSystems, typeId) ||
			Container::Contains(m_eventSystems, typeId) ||
			Container::Contains(m_imGuiSystems, typeId) ||
			Container::Contains(m_lateUpdateSystems, typeId))
			return true;

		return false;
	}

	void Systems::ImGuiUpdates()
	{
		for (auto& system : m_imGuiSystems)
		{
			system.system->OnImGui();
		}
	}

	World* Systems::CreateWorld()
	{
		return CreateWorld("World_" + std::to_string(Instance()->m_worlds.pointers.size()));
	}

	World* Systems::CreateWorld(std::string name)
	{
		//To do error message if twice same name
		World* world = new World(name);
		Systems::Instance()->m_worlds.Push(world);
		return world;
	}

	void Systems::DestroyWorld(std::string name)
	{
		//To do, queue up the destruction process.
		Systems::Instance()->m_worlds.Destroy(name);
	}

	void Systems::SetMainCamera(Camera* camera)
	{
		auto instance = Instance();
		for (auto& world : instance->m_worlds.pointers)
		{
			auto view = world->registry.view<Camera>();
			for (auto& camera : view)
			{
				view.get<Camera>(camera).isMain = false;
			}
		}
		camera->isMain = true;
		instance->m_mainCamera = camera;
	}

	void Systems::SetMainWorld(World* world)
	{
		auto instance = Instance();
		instance->m_worlds.mainWorld = world;
	}

	void Systems::SetFixedUpdateTime(float seconds)
	{
		Time::fixedDelta = seconds;
	}

	void Systems::SetTimeScale(float scale)
	{
		Time::timeScale = scale;
	}

	World* Systems::GetWorld(std::string name)
	{
		return Systems::Instance()->m_worlds.Get(name);
	}

	World* Systems::GetMainWorld()
	{
		//To do, error handling
		return Systems::Instance()->m_worlds.mainWorld;
	}

	std::vector<World*>& Systems::GetWorlds()
	{
		return Systems::Instance()->m_worlds.pointers;
	}

	Camera* Systems::GetMainCamera()
	{
		return Instance()->m_mainCamera;
	}

	Vec2f Systems::GetMouseWorldPos()
	{
		Camera* camera = Instance()->m_mainCamera;
		if (!camera)
		{
			LOG_WARN("Systems::GetMouseWorldPos, no main camera, Vec2f(0, 0) returned");
			return Vec2f(0, 0);
		}
		return camera->ScreenToWorld(Mouse::GetPosition(), Window::GetSize());
	}

	//////////////
	/// Worlds ///
	//////////////

	void Systems::Worlds::Push(World* world)
	{
		pointers.emplace_back(world);
		names.emplace_back(world->GetName());
		if (pointers.size() == 1)
			mainWorld = world;
	}

	void Systems::Worlds::Destroy(std::string name)
	{
		int64_t index = Container::FindIndex(names, name);
		Container::RemoveAt(names, index);
		delete pointers[index];
		Container::RemoveAt(pointers, index);
	}

	World* Systems::Worlds::Get(std::string name)
	{
		int64_t index = Container::FindIndex(names, name);
		return pointers[index];
	}

}

