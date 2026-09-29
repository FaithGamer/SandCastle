#include "pch.h"

#include "SandCastle/Render/LoadingScreen.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/Core/Math.h"
#include <algorithm>

namespace SandCastle
{
	namespace
	{
		using SteadyClock = std::chrono::steady_clock;

		//A report closer than this to the last frame only records the progress.
		constexpr auto frameInterval = std::chrono::milliseconds(16);

		//Input the player gave while loading, dropped when the bar closes. Window, quit
		//and device events are not in it: a gamepad plugged in during the load still has
		//to be opened, a close request still has to close the game. Mouse motion is: the
		//cursor's position is polled, not read from these events, and nudging the mouse
		//or a trackpad while waiting mustn't flip a gamepad player into mouse mode.
		constexpr Uint32 droppedInput[] =
		{
			SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP, SDL_EVENT_TEXT_EDITING, SDL_EVENT_TEXT_INPUT,
			SDL_EVENT_MOUSE_MOTION, SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_EVENT_MOUSE_BUTTON_UP, SDL_EVENT_MOUSE_WHEEL,
			SDL_EVENT_GAMEPAD_BUTTON_DOWN, SDL_EVENT_GAMEPAD_BUTTON_UP, SDL_EVENT_GAMEPAD_AXIS_MOTION
		};

		//Main thread only: the engine boots on it and nothing reports from elsewhere.
		struct LoadingState
		{
			bool active = false;
			bool complete = false;
			//The window was resizable when the bar opened: resizing stays off until the
			//load is complete, since a window maximized meanwhile stays maximized through
			//a game locking its size during its own init.
			bool resizeLocked = false;
			LoadingScreenSettings settings;
			float progress = 0.f;
			SteadyClock::time_point lastFrame;
			//Keys held when the bar closed: the engine reads no key.repeat, so their
			//auto-repeat would reach the game as fresh presses until they're released.
			std::vector<SDL_Scancode> heldKeys;
		};
		LoadingState loading;

		//Waits for the render thread while pumping the window's events, where a plain
		//WorkerThread::Wait would not: a swap held on a message to the window's thread
		//(an overlay's hook, a present routed through DXGI) must not wait on it forever.
		void WaitPumping(WorkerThread& thread)
		{
			while (thread.HaveTask())
			{
				SDL_PumpEvents();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}

	void LoadingScreen::Begin(const LoadingScreenSettings& settings)
	{
		if (!settings.enabled)
			return;
		loading.active = true;
		loading.settings = settings;
		SDL_Window* window = Window::GetSDLWindow();
		loading.resizeLocked = (SDL_GetWindowFlags(window) & SDL_WINDOW_RESIZABLE) != 0;
		if (loading.resizeLocked)
			SDL_SetWindowResizable(window, false);
		Present();
	}

	void LoadingScreen::Report(float progress)
	{
		if (!loading.active || loading.complete)
			return;
		loading.progress = Math::Max(loading.progress, Math::Clamp01(progress));
		if (SteadyClock::now() - loading.lastFrame >= frameInterval)
			Present();
	}

	void LoadingScreen::Complete()
	{
		if (!loading.active || loading.complete)
			return;
		loading.complete = true;
		loading.progress = 1.f;
		//The full bar waits its turn behind a frame still in flight, then is waited for
		//itself: it is on screen, and the render thread idle, when the boot moves on.
		auto& thread = Renderer2D::Instance()->m_queue.thread;
		WaitPumping(thread);
		Present();
		WaitPumping(thread);
		if (loading.resizeLocked)
		{
			SDL_SetWindowResizable(Window::GetSDLWindow(), true);
			loading.resizeLocked = false;
		}
	}

	void LoadingScreen::End()
	{
		if (!loading.active)
			return;
		loading.active = false;
		SDL_PumpEvents();
		for (Uint32 type : droppedInput)
			SDL_FlushEvent(type);
		int count = 0;
		const bool* keys = SDL_GetKeyboardState(&count);
		for (int i = 0; i < count; i++)
		{
			if (keys[i])
				loading.heldKeys.push_back((SDL_Scancode)i);
		}
	}

	bool LoadingScreen::HoldsBack(const SDL_Event& event)
	{
		if (loading.heldKeys.empty())
			return false;
		if (event.type != SDL_EVENT_KEY_DOWN && event.type != SDL_EVENT_KEY_UP)
			return false;
		auto key = std::find(loading.heldKeys.begin(), loading.heldKeys.end(), event.key.scancode);
		if (key == loading.heldKeys.end())
			return false;
		if (event.type == SDL_EVENT_KEY_UP)
			loading.heldKeys.erase(key);
		return true;
	}

	void LoadingScreen::Present()
	{
		loading.lastFrame = SteadyClock::now();
		SDL_PumpEvents();
		auto renderer = Renderer2D::Instance();
		//Never queue behind a frame that isn't out yet (a swap held by the driver).
		if (renderer->m_queue.thread.HaveTask())
			return;
		//Renderer2D::Process's rule: nothing is drawn into a minimized window.
		if (!Window::GetRenderWhenMinimized() && Window::GetMinimized())
			return;
		auto& settings = loading.settings;
		renderer->m_queue.thread.Queue(&Renderer2D::LoadingFrameThread, renderer.get(),
			Window::GetSize(), loading.progress, settings.backgroundColor, settings.barColor);
	}
}
