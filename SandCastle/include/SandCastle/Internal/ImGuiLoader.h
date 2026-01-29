#pragma once
#include "SandCastle/Core/Vec.h"
#include <mutex>
#include <SDL3/SDL.h>


namespace SandCastle
{
	class ImGuiLoader
	{
	public:
		static void BeginImGui();
		static bool Events(SDL_Event& event);
		static void EndImGui(Vec2u windowSize);
		static void LoadImGui(SDL_Window* sdlWindow, SDL_GLContext sdlGlContext, bool lightTheme = false);
		static void ExitImGui();
		static std::atomic<bool> enabled;
		static std::mutex mutex;
	};
}