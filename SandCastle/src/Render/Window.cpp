#include "pch.h"

#include "SandCastle/Core/Log.h"
#include "SandCastle/Render/Window.h"


namespace SandCastle
{
	Window::~Window()
	{
		SDL_GL_DestroyContext(m_glContext);
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

		m_size = size;

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
		m_glContext = SDL_GL_CreateContext(m_window);
		ASSERT_LOG_ERROR(m_glContext, LogSDLError("Cannot create OpenGL Context"));

		//Loading OpenGL Functions addresses
		bool loadGlad = (bool)gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
		ASSERT_LOG_ERROR(loadGlad, "Couldn't initialize GLAD");

		//Logging additional information
		auto c = glGetString(GL_VENDOR);
		LOG_INFO("OpenGL Loaded.");
		LOG_INFO("Version: " + std::string((const char*)glGetString(GL_VERSION)));
		LOG_INFO("Renderer: " + std::string((const char*)glGetString(GL_RENDERER)));
		LOG_INFO("Vendor: " + std::string((const char*)glGetString(GL_VENDOR)));

		int maxVertAttrib = 0;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertAttrib);

		LOG_INFO("Max. Vertex attributes: " + std::to_string(maxVertAttrib));

		//Viewport size and clear color
		glViewport(0, 0, size.x, size.y);

		//Enabling blending
		glEnable(GL_BLEND);

		//Enabling depth test
		glEnable(GL_DEPTH_TEST);

		//Standard blending parameters for most case uses
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		SDL_GL_SetSwapInterval(0);

		m_clearColor = Vec4f(0.1, 0.1, 0.1, 1);

		m_initialized = true;
	}

	void Window::SetWindowSize(Vec2u size)
	{
		Window::Instance()->SetSize(size);
	}

	void Window::SetFullScreen(bool fullscreen)
	{
		SDL_Window* window = Instance()->m_window;
		//Borderless by default
		SDL_SetWindowFullscreen(window, fullscreen);

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

	void Window::SetSize(float width, float height)
	{
		Window::Instance()->SetSize(Vec2u((unsigned int)width, (unsigned int)height));
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

	Vec2u Window::GetSize()
	{
		return Window::Instance()->m_size;
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
		return (float)window->m_size.x / (float)window->m_size.y;
	}

	SDL_GLContext Window::GetSDL_GLContext()
	{
		return Window::Instance()->m_glContext;
	}

	SDL_Window* Window::GetSDLWindow()
	{
		return Window::Instance()->m_window;
	}

	SignalSender<Vec2u>* Window::GetResizeSignal()
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
	SignalSender<bool>* Window::GetFocusSignal()
	{
		return &Window::Instance()->FocusSignal;
	}

	SignalSender<bool>* Window::GetMinimizedSignal()
	{
		return &Window::Instance()->MinimizedSignal;
	}

	void Window::Clear()
	{
		Bind();
		glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void Window::SetSize(Vec2u size)
	{
		SDL_SetWindowSize(m_window, size.x, size.y);
		glViewport(0, 0, size.x, size.y);
		m_size = size;
		ResizeSignal.SendSignal(m_size);
	}

	void Window::Bind()
	{
		glViewport(0, 0, m_size.x, m_size.y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
