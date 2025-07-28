#pragma once

#include <string>
#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Core/Vec.h"
#include "RenderTarget.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	/// @brief The SDL window and OpenGL context
	class Window : public Singleton<Window>, public RenderTarget
	{
	public:
		~Window();
		static void SetWindowSize(Vec2u size);
		static void SetFullScreen(bool fullscreen);
		static void SetVsync(bool vsync);
		static void ClearWindow();
		static void RenderWindow();
		static void SetSize(float width, float height);
		static void SetClearColor(Vec4f color);
		static void ShowCursor(bool showCursor);
		static void SetRenderWhenMinimized(bool renderWhenMinimized);
		static bool IsInitialized();
		/// @brief Get the V sync mode
		/// @param r the v sync mode output
		/// @return True if suceed, false if error.
		static bool GetVSync(int* r);
		static void Maximize();
		/// @brief Makes no difference between fullscren and desktop fullscreen
		/// @return True if window is fullscreen
		static bool GetFullScreen();
		static Vec2u GetSize();
		/// @brief Return the monitor resolution, if multiple monitors, return the monitor where the center of the window lies.
		/// @return Screen resolution
		static Vec2i GetScreenSize();
		static float GetAspectRatio();
		static SDL_GLContext GetSDL_GLContext();
		static SDL_Window* GetSDLWindow();
		static SignalSender<Vec2u>* GetResizeSignal();
		static bool GetRenderWhenMinimized();
		static bool GetMinimized();
		static bool GetFocus();
		/// @brief Send true when focus over window is gained, false when lost
		static SignalSender<bool>* GetFocusSignal();
		static SignalSender<bool>* GetMinimizedSignal();

		void SetSize(Vec2u size) override;
		void Bind() override;
		void Clear() override;
		
		SignalSender<Vec2u> ResizeSignal;
		SignalSender<bool> FocusSignal;
		SignalSender<bool> MinimizedSignal;
	private:
		friend Engine;
		friend Singleton<Window>;
		Window() = default;
		void Init(std::string name, Vec2u size);
		bool m_initialized = false;
		bool m_renderWhenMiminized = true;
		SDL_Window* m_window = nullptr;
		SDL_GLContext m_glContext = nullptr;
		Vec4f m_clearColor = { 0, 0, 0, 1 };

		Vec2u m_size = { 100, 100 };
	};
}
