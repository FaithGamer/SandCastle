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
		/// @brief Choose how fullscreen is presented. Disabled (default) uses a
		/// real borderless-desktop fullscreen the driver may promote to
		/// independent flip — fast, but DWM's cached copy of the window freezes,
		/// so screenshots, Alt-Tab thumbnails and capture tools show a stale
		/// frame until the window is recomposited. Enabled instead uses a
		/// borderless window at the exact native size but shifted 1px in
		/// position, so its rectangle no longer matches the monitor exactly —
		/// that keeps it out of independent flip and on the DWM composited path
		/// so captures stay live, with no resize (the render is unchanged), at
		/// the cost of the flip fast-path and a 1px desktop strip at the screen
		/// edge. Safe to toggle at runtime.
		static void SetCompositedFullscreen(bool enabled);
		static void SetVsync(bool vsync);
		static void ClearWindow();
		static void RenderWindow();
		static void SetClearColor(Vec4f color);
		static void ShowCursor(bool showCursor);
		/// @brief Set a custom cursor from a BMP file. The OS cursor is hidden
		/// over the window and the engine draws the cursor texture itself as a
		/// quad in the render thread, just before the swap. This works around
		/// the known WGC-vs-OpenGL bug where the OS cursor becomes invisible to
		/// the user in borderless fullscreen while a capture session is active
		/// (OBS, Streamlabs...), at the cost of one frame of cursor latency.
		/// hotX/hotY are the cursor hotspot in pixels from the top-left of the image.
		/// Pass an empty string to restore the default OS cursor.
		static void SetCursor(const std::string& texturePath, int hotX = 0, int hotY = 0);
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
		static Signal<Vec2u>* GetResizeSignal();
		/// @brief Send true when focus over window is gained, false when lost
		static Signal<bool>* GetFocusSignal();
		static Signal<bool>* GetMinimizedSignal();

		void SetSize(Vec2u size) override;
		void Bind() override;
		void Clear() override;
		void OnSDLPixelSizeChanged(SDL_Event& event);
		void OnSDLWindowResized(SDL_Event& event);
		/// @brief Called once per frame on the render thread by Renderer2D,
		/// right before SDL_GL_SwapWindow. Draws the custom cursor on top of
		/// the final framebuffer if one was set via SetCursor.
		void RenderCursorOverlay();

		Signal<Vec2u> ResizeSignal;
		Signal<bool> FocusSignal;
		Signal<bool> MinimizedSignal;
	private:
		friend Engine;
		friend Singleton<Window>;
		Window() = default;
		void Init(std::string name, Vec2u size);
		// Apply m_fullscreen / m_compositedFullscreen to the SDL window.
		void ApplyWindowMode();
		void RefreshCursor();
		void OnCursorResize(Vec2u size);
		void UploadCursorTexture(SDL_Surface* rgbaSurface);
		void DestroyCursorTexture();
		bool m_initialized = false;
		bool m_renderWhenMiminized = false;
		bool m_fullscreen = false;
		bool m_compositedFullscreen = false;
		Vec2u m_windowedSize;
		SDL_Window* m_window = nullptr;
		SDL_GLContext m_initContext = nullptr;
		SDL_GLContext m_renderContext = nullptr;
		std::string m_cursorPath;
		int m_cursorHotX = 0;
		int m_cursorHotY = 0;
		Vec4f m_clearColor = { 0, 0, 0, 1 };
		Vec2u m_pixelSize;
		// Custom-cursor GL state. Texture uploaded from main thread; shader
		// and VAO/VBO lazily created on the render thread on first draw.
		GLuint m_cursorTex = 0;
		int m_cursorTexWidth = 0;
		int m_cursorTexHeight = 0;
		int m_cursorScale = 1;
		GLuint m_cursorShader = 0;
		GLuint m_cursorVao = 0;
		GLuint m_cursorVbo = 0;
		GLint m_cursorUTransform = -1;
	};
}
