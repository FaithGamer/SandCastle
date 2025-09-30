#include "pch.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_opengl3.h>

#include "SandCastle/Internal/ImGuiLoader.h"


namespace SandCastle
{
	std::atomic<bool> ImGuiLoader::enabled = true;
	std::mutex ImGuiLoader::mutex;
	void ImGuiLoader::LoadImGui(SDL_Window* sdlWindow, SDL_GLContext sdlGlContext, bool lightTheme)
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//	io.IniFilename = NULL;
		//	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // Imgui does not control mouse visibility

		// Setup Dear ImGui style
		if (!lightTheme)
			ImGui::StyleColorsDark();
		else
			ImGui::StyleColorsLight();

		// Setup Platform/Renderer backends
		ImGui_ImplSDL3_InitForOpenGL(sdlWindow, sdlGlContext);
		ImGui_ImplOpenGL3_Init("#version 130");
	}

	void ImGuiLoader::ExitImGui()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLoader::BeginImGui()
	{
		std::lock_guard lock(mutex);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLoader::Events(SDL_Event& event)
	{
		if (event.type == SDL_EVENT_KEY_DOWN)
		{
			if (event.key.scancode == SDL_SCANCODE_F10)
				ImGuiLoader::enabled = !ImGuiLoader::enabled;
		}
		if (!ImGuiLoader::enabled)
			return;
		std::lock_guard lock(ImGuiLoader::mutex);
		ImGui_ImplSDL3_ProcessEvent(&event);
	}

	void ImGuiLoader::EndImGui(Vec2u windowSize)
	{
		std::lock_guard lock(mutex);
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)windowSize.x, (float)windowSize.y);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}