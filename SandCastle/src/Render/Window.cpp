#include "pch.h"

#include "SandCastle/Core/Log.h"
#include "SandCastle/Render/Window.h"


namespace SandCastle
{
	Window::~Window()
	{
		SDL_GL_DestroyContext(m_initContext);
		SDL_GL_DestroyContext(m_renderContext);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}
	void Window::Init(std::string name, Vec2u size)
	{
		//Initializing SDL
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD))
		{
			LOG_ERROR("Couldn't initialize SDL, error: {0}", SDL_GetError());
		}

		//Load default OpenGL library
		SDL_GL_LoadLibrary(NULL);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

		//Creating SDL Window
		m_window = SDL_CreateWindow(name.c_str(), size.x, size.y,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		ASSERT_LOG_ERROR(m_window, LogSDLError("Cannot create SDL window"));

		//Creating OpenGL Context with SDL
		m_renderContext = SDL_GL_CreateContext(m_window);
		m_initContext = SDL_GL_CreateContext(m_window);
		//SDL_GL_MakeCurrent(m_window, nullptr);
		ASSERT_LOG_ERROR(m_renderContext, LogSDLError("Cannot create render OpenGL Context"));
		ASSERT_LOG_ERROR(m_initContext, LogSDLError("Cannot create init OpenGL Context"));

		m_clearColor = Vec4f(1.1, 0.1, 0.1, 1);
		m_initialized = true;
		m_pixelSize = GetSize();
	}

	void Window::SetSize(Vec2u size)
	{
		if (!SDL_SetWindowSize(Instance()->m_window, size.x, size.y))
		{
			LOG_ERROR("Window SetSize({0}, {1}) failed: {2}", size.x, size.y, SDL_GetError());
		}
		m_pixelSize = GetSize();
	}

	void Window::SetFullScreen(bool fullscreen)
	{
		SDL_Window* window = Instance()->m_window;
		//Borderless by default
		if(!SDL_SetWindowFullscreen(window, fullscreen))
		{
			LOG_ERROR("Window fullscreen({0}) failed: {1}",fullscreen, SDL_GetError());
		}
	}

	void Window::SetVsync(bool vsync)
	{
		if (!vsync)
		{
			SDL_GL_SetSwapInterval(0);
			return;
		}

		if (SDL_GL_SetSwapInterval(-1) != 0)
		{
			LOG_WARN("Adaptative V-sync unsupported.");
			if (SDL_GL_SetSwapInterval(1) != 0)
			{
				LOG_WARN("V-sync unsupported.");
				SDL_GL_SetSwapInterval(0);
			}
		}
	}

	void Window::ClearWindow()
	{
		Window::Instance()->Clear();
	}

	void Window::RenderWindow()
	{
		SDL_GL_SwapWindow(Window::Instance()->m_window);
	}

	void Window::SetSize(unsigned int width, unsigned int height)
	{
		Window::Instance()->SetSize(Vec2u(width, height));
	}

	void Window::SetClearColor(Vec4f color)
	{
		Instance()->m_clearColor = color;
	}

	void Window::ShowCursor(bool showCursor)
	{
		if (showCursor)
			SDL_ShowCursor();
		else
			SDL_HideCursor();
	}

	void Window::SetRenderWhenMinimized(bool renderWhenMinimized)
	{
		auto i = Instance();
		i->m_renderWhenMiminized = renderWhenMinimized;
	}

	bool Window::IsInitialized()
	{
		return Window::Instance()->m_initialized;
	}

	bool Window::GetVSync(int* r)
	{
		if (SDL_GL_GetSwapInterval(r))
		{
			return true;
		}
		else
		{
			LOG_ERROR("Couldn't get the V-SYNC mode.");
			return false;
		}
	}

	void Window::Maximize()
	{
		SDL_MaximizeWindow(Instance()->m_window);
	}

	bool Window::GetFullScreen()
	{
		auto i = Instance();
		SDL_WindowFlags flags = SDL_GetWindowFlags(i->m_window);
		if ((flags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN)
			return true;
		return false;
	}

	Vec2i Window::GetSize()
	{
		int w = 0, h = 0;
		SDL_GetWindowSizeInPixels(Instance()->m_window, &w, &h);
		return Vec2i(w, h);
	}
	Vec2i Window::GetPointSize()
	{
		int w = 0, h = 0;
		SDL_GetWindowSize(Instance()->m_window, &w, &h);
		return Vec2i(w, h);
	}
	bool Window::PixelMatchPoint()
	{
		return GetPointSize() == GetSize();
	}
	Vec2i Window::GetScreenSize()
	{
		int index = SDL_GetDisplayForWindow(Instance()->m_window);
		const SDL_DisplayMode* dp = SDL_GetDesktopDisplayMode(index);
		return Vec2i(dp->w, dp->h);
	}
	float Window::GetAspectRatio()
	{
		auto window = Window::Instance();
		return (float)window->m_pixelSize.x / (float)window->m_pixelSize.y;
	}

	SDL_GLContext Window::GetInitContext()
	{
		return Window::Instance()->m_initContext;
	}

	SDL_GLContext Window::GetRenderContext()
	{
		return Window::Instance()->m_renderContext;
	}

	SDL_Window* Window::GetSDLWindow()
	{
		return Window::Instance()->m_window;
	}

	Signal<Vec2u>* Window::GetResizeSignal()
	{
		return &Window::Instance()->ResizeSignal;
	}

	bool Window::GetRenderWhenMinimized()
	{
		return Instance()->m_renderWhenMiminized;
	}

	bool Window::GetMinimized()
	{
		SDL_WindowFlags flags = SDL_GetWindowFlags(Instance()->m_window);

		return (flags & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
	}

	bool Window::GetFocus()
	{
		SDL_WindowFlags flags = SDL_GetWindowFlags(Instance()->m_window);

		return (flags & SDL_WINDOW_MOUSE_FOCUS) == SDL_WINDOW_MOUSE_FOCUS;

	}
	Signal<bool>* Window::GetFocusSignal()
	{
		return &Window::Instance()->FocusSignal;
	}

	Signal<bool>* Window::GetMinimizedSignal()
	{
		return &Window::Instance()->MinimizedSignal;
	}

	void Window::Clear()
	{
		Bind();
		glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void Window::OnSDLPixelSizeChanged(SDL_Event& event)
	{
	/*	glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, event.window.data1, event.window.data2);
		m_pixelSize.x = (unsigned int)event.window.data1;
		m_pixelSize.y = (unsigned int)event.window.data2;
		ResizeSignal.Send(m_pixelSize);*/
		//LOG_INFO("pixelizes. [{0}, {1}]", m_pixelSize.x, m_pixelSize.y);
	}

	void Window::OnSDLWindowResized(SDL_Event& event)
	{
		//Nothing for now, we only care about pixel size as far as I am aware of.
		int w, h;
		SDL_GetWindowSize(Instance()->m_window, &w, &h);
		//LOG_INFO("resized. [{0}, {1}]", w, h);
	}

	void Window::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, m_pixelSize.x, m_pixelSize.y);
	}
}
