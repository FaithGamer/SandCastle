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
		/// @brief Size in point, pixel size is not guaranteed to match the point size
		/// depending on hardware and OS settings.
		/// PixelMatchPoint() to know if pixel match points.
		static void SetSize(unsigned int width, unsigned int height);
		static void SetFullScreen(bool fullscreen);
		static void SetVsync(bool vsync);
		static void ClearWindow();
		static void RenderWindow();
		static void SetClearColor(Vec4f color);
		static void ShowCursor(bool showCursor);
		static void SetRenderWhenMinimized(bool renderWhenMinimized);
		static bool IsInitialized();
		/// @brief Get the V sync mode
		/// @param r the v sync mode output
		/// @return True if suceed, false if error.
		static bool GetVSync(int* r);
		/// @brief Window will take as much space as possible.
		static void Maximize();
		/// @brief Makes no difference between fullscren and desktop fullscreen
		/// @return True if window is fullscreen
		static bool GetFullScreen();
		/// @brief Get pixel size of the render area
		static Vec2i GetSize();
		/// @brief Get size of the window in point for display with higher dpi density
		/// (retina screen, or windows using 150% scaling factor)
		static Vec2i GetPointSize();
		/// @brief Depending on OS and Hardware, point and pixel size may differ.
		/// @return true if pixel size match point size.
		static bool PixelMatchPoint();
		/// @brief Return the monitor resolution, if multiple monitors, return the monitor where the center of the window lies.
		/// @return Screen resolution
		static Vec2i GetScreenSize();
		static float GetAspectRatio();
		static SDL_GLContext GetInitContext();
		static SDL_GLContext GetRenderContext();
		static SDL_Window* GetSDLWindow();
		static bool GetRenderWhenMinimized();
		static bool GetMinimized();
		static bool GetFocus();
		/// @brief Fire when the size of the window (in pixel) changed.
		/// I'm still trying to figure out if there's some edge cases where the point
		/// size of the window change without the pixel size, but as far as I'm aware of
		/// this shouldn't be a situation that you should worry about.
		/// I don't even know if point size is useful at any point from the gamedev stand point.
		/// @return 
		static SignalSender<Vec2u>* GetResizeSignal();
		/// @brief Send true when focus over window is gained, false when lost
		static SignalSender<bool>* GetFocusSignal();
		static SignalSender<bool>* GetMinimizedSignal();

		void SetSize(Vec2u size) override;
		void Bind() override;
		void Clear() override;
		void OnSDLPixelSizeChanged(SDL_Event& event);
		void OnSDLWindowResized(SDL_Event& event);
		
		SignalSender<Vec2u> ResizeSignal;
		SignalSender<bool> FocusSignal;
		SignalSender<bool> MinimizedSignal;
	private:
		friend Engine;
		friend Singleton<Window>;
		Window() = default;
		void Init(std::string name, Vec2u size);
		bool m_initialized = false;
		bool m_renderWhenMiminized = false;
		SDL_Window* m_window = nullptr;
		SDL_GLContext m_initContext = nullptr;
		SDL_GLContext m_renderContext = nullptr;
		Vec4f m_clearColor = { 0, 0, 0, 1 };
		Vec2u m_pixelSize;
	};
}
