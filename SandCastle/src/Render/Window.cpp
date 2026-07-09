#include "pch.h"

#include "SandCastle/Core/Log.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/Core/Math.h"

namespace SandCastle
{
	Window::~Window()
	{
		// Cursor GL resources (texture/shader/VAO/VBO) live on the render
		// context. By the time the Window singleton runs its destructor the
		// render thread is already gone, so we deliberately leak them — the
		// process is exiting anyway and the driver reclaims everything.
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

		m_clearColor = Vec4f(0.1, 0.1, 0.1, 1);
		m_initialized = true;
		m_pixelSize = GetSize();
		m_windowedSize = size;
		ResizeSignal.Listen(&Window::OnCursorResize, this);
	}

	void Window::SetSize(Vec2u size)
	{
		if (!SDL_SetWindowSize(Instance()->m_window, size.x, size.y))
		{
			LOG_ERROR("Window SetSize({0}, {1}) failed: {2}", size.x, size.y, SDL_GetError());
		}
		m_pixelSize = GetSize();
		if (!m_fullscreen)
			m_windowedSize = size;
	}

	void Window::SetFullScreen(bool fullscreen)
	{
		auto self = Instance();
		self->m_fullscreen = fullscreen;
		self->ApplyWindowMode();
	}

	void Window::SetCompositedFullscreen(bool enabled)
	{
		auto self = Instance();
		if (self->m_compositedFullscreen == enabled)
			return;
		self->m_compositedFullscreen = enabled;
		// Re-apply live so toggling it during a run switches style immediately.
		if (self->m_fullscreen)
			self->ApplyWindowMode();
	}

	void Window::ApplyWindowMode()
	{
		SDL_Window* window = m_window;

		if (!m_fullscreen)
		{
			// Windowed: leave any fullscreen style and restore the border plus
			// the last windowed size (the composited path below resizes the
			// window, so we can't rely on SDL remembering it).
			SDL_SetWindowFullscreen(window, false);
			SDL_SetWindowBordered(window, true);
			SDL_SetWindowSize(window, m_windowedSize.x, m_windowedSize.y);
			return;
		}

		if (!m_compositedFullscreen)
		{
			// Fast path: real borderless-desktop fullscreen. The driver may
			// promote this to independent flip (direct scanout), which freezes
			// DWM's cached copy of the window so screenshots/thumbnails go stale.
			if (!SDL_SetWindowFullscreen(window, true))
				LOG_ERROR("Window fullscreen(true) failed: {0}", SDL_GetError());
			return;
		}

		// Capture-friendly fullscreen: a borderless window at the EXACT native
		// size (so the engine renders identically to real fullscreen — any size
		// mismatch folds the render), but shifted 1px up so the window rectangle
		// doesn't exactly match the monitor. Independent flip requires an exact
		// screen-matching rect, so this shift alone keeps the window on the DWM
		// composited path and screen-capture reads live frames. The top row
		// spills 1px off-screen and a 1px desktop strip shows at the bottom.
		SDL_SetWindowFullscreen(window, false);
		SDL_DisplayID display = SDL_GetDisplayForWindow(window);
		SDL_Rect bounds{};
		if (!SDL_GetDisplayBounds(display, &bounds))
		{
			LOG_ERROR("Composited fullscreen: SDL_GetDisplayBounds failed: {0}", SDL_GetError());
			// Don't leave a half-configured window — fall back to normal fullscreen.
			if (!SDL_SetWindowFullscreen(window, true))
				LOG_ERROR("Window fullscreen(true) fallback failed: {0}", SDL_GetError());
			return;
		}
		SDL_SetWindowBordered(window, false);
		SDL_SetWindowSize(window, bounds.w, bounds.h+2);
		SDL_SetWindowPosition(window, bounds.x, bounds.y);
		SDL_RaiseWindow(window);
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

	void Window::SetCursor(const std::string& texturePath, int hotX, int hotY)
	{
		auto instance = Instance();

		if (texturePath.empty())
		{
			instance->m_cursorPath.clear();
			instance->DestroyCursorTexture();
			SDL_ShowCursor();
			SDL_SetCursor(SDL_GetDefaultCursor());
			return;
		}

		instance->m_cursorPath = texturePath;
		instance->m_cursorHotX = hotX;
		instance->m_cursorHotY = hotY;
		instance->RefreshCursor();
		// Hide the OS cursor over the window: we draw our own each frame.
		SDL_HideCursor();
	}

	void Window::RefreshCursor()
	{
		if (m_cursorPath.empty())
			return;

		SDL_Surface* surface = SDL_LoadBMP(m_cursorPath.c_str());
		if (!surface)
		{
			LOG_ERROR("Window::SetCursor: failed to load '{0}': {1}", m_cursorPath, SDL_GetError());
			return;
		}

		int h = GetSize().y;
		int scale = h < 720 ? 1 : h < 1080 ? 2 : h < 1440 ? 3 : 4;

		SDL_Surface* scaled = SDL_ScaleSurface(surface, surface->w * scale, surface->h * scale, SDL_SCALEMODE_NEAREST);
		SDL_DestroySurface(surface);
		if (!scaled)
		{
			LOG_ERROR("Window::SetCursor: SDL_ScaleSurface failed: {0}", SDL_GetError());
			return;
		}

		m_cursorScale = scale;
		UploadCursorTexture(scaled);
		SDL_DestroySurface(scaled);
	}

	void Window::UploadCursorTexture(SDL_Surface* surface)
	{
		if (!surface)
			return;
		// Normalize to RGBA8 so glTexImage2D upload is straightforward.
		SDL_Surface* rgba = surface;
		bool ownsRgba = false;
		if (surface->format != SDL_PIXELFORMAT_RGBA32)
		{
			rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
			if (!rgba)
			{
				LOG_ERROR("Window::UploadCursorTexture: SDL_ConvertSurface failed: {0}", SDL_GetError());
				return;
			}
			ownsRgba = true;
		}

		if (m_cursorTex == 0)
			glGenTextures(1, &m_cursorTex);
		glBindTexture(GL_TEXTURE_2D, m_cursorTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rgba->w, rgba->h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
		// Make sure the render thread will see the upload.
		glFinish();
		m_cursorTexWidth = rgba->w;
		m_cursorTexHeight = rgba->h;

		if (ownsRgba)
			SDL_DestroySurface(rgba);
	}

	void Window::DestroyCursorTexture()
	{
		if (m_cursorTex != 0)
		{
			glDeleteTextures(1, &m_cursorTex);
			m_cursorTex = 0;
		}
		m_cursorTexWidth = 0;
		m_cursorTexHeight = 0;
	}

	void Window::OnCursorResize(Vec2u size)
	{
		RefreshCursor();
	}

	void Window::RenderCursorOverlay()
	{
		if (m_cursorTex == 0)
			return;
		if (GetMinimized())
			return;

		// Lazy GL init on the render thread. VAOs and program objects must be
		// created in the context that will use them (m_renderContext).
		if (m_cursorShader == 0)
		{
			static const char* kVs =
				"#version 330 core\n"
				"layout(location=0) in vec2 aPos;\n"
				"layout(location=1) in vec2 aUv;\n"
				"uniform vec4 uTransform; // xy = offset (NDC), zw = scale (NDC)\n"
				"out vec2 vUv;\n"
				"void main(){\n"
				"  vec2 p = aPos * uTransform.zw + uTransform.xy;\n"
				"  vUv = aUv;\n"
				"  gl_Position = vec4(p, 0.0, 1.0);\n"
				"}\n";
			static const char* kFs =
				"#version 330 core\n"
				"in vec2 vUv;\n"
				"uniform sampler2D uTex;\n"
				"out vec4 fColor;\n"
				"void main(){\n"
				"  vec4 c = texture(uTex, vUv);\n"
				"  if (c.a < 0.01) discard;\n"
				"  fColor = c;\n"
				"}\n";

			auto compile = [](GLenum stage, const char* src) -> GLuint {
				GLuint s = glCreateShader(stage);
				glShaderSource(s, 1, &src, nullptr);
				glCompileShader(s);
				GLint ok = GL_FALSE;
				glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
				if (!ok)
				{
					char log[1024]{};
					glGetShaderInfoLog(s, sizeof(log), nullptr, log);
					LOG_ERROR("Window cursor shader compile failed: {0}", log);
					glDeleteShader(s);
					return 0;
				}
				return s;
			};

			GLuint vs = compile(GL_VERTEX_SHADER, kVs);
			GLuint fs = compile(GL_FRAGMENT_SHADER, kFs);
			if (!vs || !fs)
			{
				if (vs) glDeleteShader(vs);
				if (fs) glDeleteShader(fs);
				return;
			}
			m_cursorShader = glCreateProgram();
			glAttachShader(m_cursorShader, vs);
			glAttachShader(m_cursorShader, fs);
			glLinkProgram(m_cursorShader);
			glDeleteShader(vs);
			glDeleteShader(fs);
			GLint linked = GL_FALSE;
			glGetProgramiv(m_cursorShader, GL_LINK_STATUS, &linked);
			if (!linked)
			{
				char log[1024]{};
				glGetProgramInfoLog(m_cursorShader, sizeof(log), nullptr, log);
				LOG_ERROR("Window cursor shader link failed: {0}", log);
				glDeleteProgram(m_cursorShader);
				m_cursorShader = 0;
				return;
			}
			m_cursorUTransform = glGetUniformLocation(m_cursorShader, "uTransform");
			GLint uTex = glGetUniformLocation(m_cursorShader, "uTex");
			glUseProgram(m_cursorShader);
			glUniform1i(uTex, 0);

			// Quad: pos.xy in [0,1], uv in [0,1]. UV V is NOT flipped: the
			// negative ndcScaleY below already inverts Y at the NDC stage, so
			// straight-through UVs give us the image right-side-up.
			const float kQuad[] = {
				// pos      uv
				0.f, 0.f,  0.f, 0.f,
				1.f, 0.f,  1.f, 0.f,
				1.f, 1.f,  1.f, 1.f,

				0.f, 0.f,  0.f, 0.f,
				1.f, 1.f,  1.f, 1.f,
				0.f, 1.f,  0.f, 1.f,
			};
			glGenVertexArrays(1, &m_cursorVao);
			glGenBuffers(1, &m_cursorVbo);
			glBindVertexArray(m_cursorVao);
			glBindBuffer(GL_ARRAY_BUFFER, m_cursorVbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(kQuad), kQuad, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
			glBindVertexArray(0);
		}

		// Mouse position is in points, not pixels. Convert to pixels using
		// window scale, since the cursor texture size is in pixels too.
		float mxPoints = 0.f, myPoints = 0.f;
		SDL_GetMouseState(&mxPoints, &myPoints);
		int pointW = 0, pointH = 0;
		SDL_GetWindowSize(m_window, &pointW, &pointH);
		int pixelW = 0, pixelH = 0;
		SDL_GetWindowSizeInPixels(m_window, &pixelW, &pixelH);
		if (pointW <= 0 || pointH <= 0 || pixelW <= 0 || pixelH <= 0)
			return;
		float sx = (float)pixelW / (float)pointW;
		float sy = (float)pixelH / (float)pointH;
		float mxPx = mxPoints * sx;
		float myPx = myPoints * sy;

		// Top-left of cursor quad in pixels (subtract hotspot, scaled the
		// same way the BMP was scaled in RefreshCursor).
		float hotPxX = (float)(m_cursorHotX * m_cursorScale);
		float hotPxY = (float)(m_cursorHotY * m_cursorScale);
		float quadPxX = mxPx - hotPxX;
		float quadPxY = myPx - hotPxY;

		// Convert pixel rect to NDC. Y is flipped because OpenGL NDC has +Y up
		// while SDL mouse coords have +Y down.
		float ndcOffsetX = (quadPxX / (float)pixelW) * 2.f - 1.f;
		float ndcOffsetY = 1.f - (quadPxY / (float)pixelH) * 2.f;
		float ndcScaleX = ((float)m_cursorTexWidth / (float)pixelW) * 2.f;
		float ndcScaleY = -((float)m_cursorTexHeight / (float)pixelH) * 2.f;

		// Set GL state. We don't need depth, and we want straight alpha
		// blending (texture pixels are unpremultiplied RGBA from SDL).
		GLboolean wasBlend = glIsEnabled(GL_BLEND);
		GLboolean wasDepth = glIsEnabled(GL_DEPTH_TEST);
		GLint prevProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
		GLint prevVao = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
		GLint prevActiveTex = 0;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTex);
		GLint prevTex0 = 0;
		glActiveTexture(GL_TEXTURE0);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex0);
		GLint prevBlendSrcRGB = 0, prevBlendDstRGB = 0, prevBlendSrcA = 0, prevBlendDstA = 0;
		glGetIntegerv(GL_BLEND_SRC_RGB, &prevBlendSrcRGB);
		glGetIntegerv(GL_BLEND_DST_RGB, &prevBlendDstRGB);
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &prevBlendSrcA);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &prevBlendDstA);

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0, 0, pixelW, pixelH);

		glUseProgram(m_cursorShader);
		glUniform4f(m_cursorUTransform, ndcOffsetX, ndcOffsetY, ndcScaleX, ndcScaleY);
		glBindVertexArray(m_cursorVao);
		glBindTexture(GL_TEXTURE_2D, m_cursorTex);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Restore.
		glBindTexture(GL_TEXTURE_2D, (GLuint)prevTex0);
		glActiveTexture((GLenum)prevActiveTex);
		glBindVertexArray((GLuint)prevVao);
		glUseProgram((GLuint)prevProgram);
		glBlendFuncSeparate(prevBlendSrcRGB, prevBlendDstRGB, prevBlendSrcA, prevBlendDstA);
		if (!wasBlend) glDisable(GL_BLEND);
		if (wasDepth) glEnable(GL_DEPTH_TEST);
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
		// Logical fullscreen state: the composited path is a borderless window,
		// not an SDL fullscreen flag, so we can't read it off SDL_GetWindowFlags.
		return Instance()->m_fullscreen;
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
		// Clear the FULL window (bars included) to the clear color, then restore
		// the letterbox scissor for the actual draws — otherwise the scissor
		// would leave last frame's content in the bars.
		glDisable(GL_SCISSOR_TEST);
		glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		ApplyScissor();
	}

	void Window::OnSDLPixelSizeChanged(SDL_Event& event)
	{
		static bool recursive = false;
		m_pixelSize.x = (unsigned int)event.window.data1;
		m_pixelSize.y = (unsigned int)event.window.data2;
		ResizeSignal.Send(m_pixelSize);
	}

	void Window::OnSDLWindowResized(SDL_Event& event)
	{
		//Nothing for now, we only care about pixel size as far as I am aware of.
	}

	void Window::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, Math::FloorToEven(m_pixelSize.x), Math::FloorToEven(m_pixelSize.y));
		ApplyScissor();
	}

	void Window::SetLetterbox(int x, int y, int width, int height)
	{
		auto ins = Instance();
		ins->m_letterboxActive = true;
		ins->m_letterboxX = x;
		ins->m_letterboxY = y;
		ins->m_letterboxW = width;
		ins->m_letterboxH = height;
	}

	void Window::ClearLetterbox()
	{
		Instance()->m_letterboxActive = false;
	}

	void Window::ApplyScissor()
	{
		if (m_letterboxActive)
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(m_letterboxX, m_letterboxY, m_letterboxW, m_letterboxH);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}
	}
}
