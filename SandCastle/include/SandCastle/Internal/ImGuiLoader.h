#pragma once
#include "SandCastle/Core/Vec.h"
#include <SDL3/SDL.h>


namespace SandCastle
{
	void BeginImGui();
	void EndImGui(Vec2u windowSize);
	void LoadImGui(SDL_Window* sdlWindow, SDL_GLContext sdlGlContext, bool lightTheme = false);
	void ExitImGui();
}