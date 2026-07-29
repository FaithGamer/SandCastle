#include "pch.h"

#include <glm/gtc/matrix_transform.hpp>
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Render/RenderTarget.h"
#include "SandCastle/Render/RenderTexture.h"
#include "SandCastle/Render/Shader.h"
#include "SandCastle/Render/RenderOptions.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/SpriteRenderSystem.h"
#include "SandCastle/Render/LineRendererSystem.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Core/Container.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Core/Print.h"
#include "SandCastle/Core/Profiling.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Core/Time.h"
#include "SandCastle/Internal/ImGuiLoader.h"

namespace SandCastle
{
	//FBO sizes are floored to even so the texture matches the even-floored
	//viewport (Window::Bind / RenderTexture::Bind). An odd-sized texture
	//sampled over an even viewport shears every composite by a fraction of
	//a pixel, which breaks pixel-perfect rendering on odd window sizes.
	static Vec2u EvenSize(Vec2i size)
	{
		return Vec2u((unsigned int)Math::FloorToEven(size.x), (unsigned int)Math::FloorToEven(size.y));
	}

	Renderer2D::Renderer2D()
	{

	}

	void Renderer2D::Init()
	{
		m_queue.thread.StartThread();
		if (m_init)
		{
			LOG_ERROR("Trying to init the renderer twice.");
			return;
		}
		m_queue.thread.Queue(&Renderer2D::InitThread, this);
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); //stupid safety to make sure the wait wall just after actually work
		Wait();
	}

	void Renderer2D::PostAssetInit()
	{
		if (m_init)
		{
			LOG_ERROR("Trying to init the renderer twice.");
			return;
		}
		m_queue.thread.Queue(&Renderer2D::PostAssetInitThread, this);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		Wait();
	}

	void Renderer2D::InitThread()
	{
		if (!SDL_GL_MakeCurrent(Window::GetSDLWindow(), Window::GetRenderContext()))
		{
			LOG_ERROR(LogSDLError("Cannot set the context."));
		}
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
		auto size = Window::GetSize();
		glViewport(0, 0, Math::FloorToEven(size.x), Math::FloorToEven(size.y));
		//glViewport(0, 0, size.x, size.y);

		//Globally enabling blending
		glEnable(GL_BLEND);

		SDL_GL_SetSwapInterval(0);
	}

	void Renderer2D::PostAssetInitThread()
	{
		ASSERT_LOG_ERROR(Window::IsInitialized(), "Cannot create Renderer2D before Window is initialized.");

		m_rendering = false;
		//Limitations
		m_maxQuads = 10000;
		m_maxVertices = m_maxQuads * 4;
		m_maxIndices = m_maxQuads * 6;

		//IndexBuffer (quads)
		uint32_t* quadIndices = new uint32_t[m_maxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < m_maxIndices; i += 6)
		{
			quadIndices[0 + i] = 0 + offset;
			quadIndices[1 + i] = 1 + offset;
			quadIndices[2 + i] = 2 + offset;
			quadIndices[3 + i] = 2 + offset;
			quadIndices[4 + i] = 3 + offset;
			quadIndices[5 + i] = 0 + offset;

			offset += 4;
		}

		m_quadIndexBuffer = makesptr<IndexBuffer>(quadIndices, m_maxIndices);
		delete[] quadIndices;

		//Camera uniform buffer
		m_sceneUniformBuffer = makesptr<UniformBuffer>(sizeof(SceneBufferData), m_sceneUniformBinding);

		m_whiteTexture = new Texture();
		m_whiteTextureID = m_whiteTexture->GetId();

		auto window = Window::Instance();

		//Create default materials
		auto defaultShader = Assets::Get<Shader>("default.shader");
		//Not using CreateMaterial because it would be blocking thread by attempting to create the batch.
		m_defaultBatchMaterial = 
			m_materials.emplace_back(new Material(defaultShader, (MaterialID)m_materials.size(), false));
		m_defaultLayerMaterial = 
			m_materials.emplace_back(new Material(Assets::Get<Shader>("default_layer.shader"), (MaterialID)m_materials.size(), true));

		m_defaultBatchMaterial->SetFloat("uDiscardAlpha", 0.05f);
		m_defaultLayerMaterial->GetRenderOptions()->SetDepthTest(false);
		m_defaultLineShader = Assets::Get<Shader>("line.shader");
		m_defaultWireShader = Assets::Get<Shader>("wire.shader");

		//Screen layer
		std::vector<Vec2f> screenSpace{ {-1, -1}, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		m_layers.push_back(RenderLayer("Window",
			0,
			window,
			m_defaultLayerMaterial,
			GenerateLayerVertexArray(screenSpace, 0)));
		//m_layerMax++;
		m_lastLayerAdded = 1;

		//Register the quad batches for every layer * material. Registration is
		//cheap (a vector slot + the material's uniform wiring); the vertex
		//buffers are allocated later, on the first quad each pair receives.
		for (auto& layer : m_layers)
		{
			for (auto& mat : m_materials)
			{
				CreateQuadBatchThread(layer, mat);
			}
		}
		//Renderer2D::CreateQuadBatchThread(m_layers.back(), m_defaultBatchMaterial);
		//SetShaderUniformSampler(m_defaultLayerMaterial->GetShader(), MAX_TEXTURE_INDEX);

		//Listen to window resize signal
		Window::GetResizeSignal()->Listen(&Renderer2D::OnWindowResize, this);

		//Set render target to be the window by default
		SetRenderTarget(window);

		m_init = true;
	}

	Renderer2D::~Renderer2D()
	{
		m_queue.thread.StopThread();
		Wait();
		//To do
		delete m_whiteTexture;
		for (int i = 0; i < m_materials.size(); i++)
		{
			delete m_materials[i];
		}
	}

	void Renderer2D::SetShaderUniformSampler(Shader* shader, uint32_t count)
	{
		std::vector<int> sampler;
		for (uint32_t i = 0; i < count; i++)
		{
			sampler.emplace_back(i);
		}
		auto location = shader->GetUniformLocation("uTextures");
		shader->SetUniformArray(location, &sampler[0], (GLsizei)sampler.size());
	}

	void Renderer2D::SetRenderTarget(sptr<RenderTarget> target)
	{
		m_target = target;
	}

	LayerID Renderer2D::AddLayer(std::string name, unsigned int height, Material* material)
	{
		auto ins = Instance();
		ASSERT_LOG_ERROR((ins->m_layers.size() < (MAX_LAYERS / 2)), "{0} layer, is over the max layer count: {1}.", name, MAX_LAYERS / 2);
		ins->m_queue.thread.Queue(&Renderer2D::AddLayerThread, ins.get(), name, height, material);
		ins->Wait();
		auto& layer = ins->m_layers[(size_t)ins->m_lastLayerAdded];
		for (auto& mat : ins->m_materials)
		{
			if (!mat->IsLayer())
				ins->m_queue.thread.Queue(&Renderer2D::CreateQuadBatchThread, ins.get(), layer, mat);
		}
		ins->Wait();
		//ins->m_layerMax++;
		return ins->m_lastLayerAdded;
	}

	void Renderer2D::SetLayerSortZ(LayerID layer, bool zsort)
	{
		Renderer2D::Instance()->m_queue.zsort[layer] = zsort;
	}

	LayerID Renderer2D::AddLayer(std::string name, Material* material)
	{
		return AddLayer(name, 0, material);
	}

	void Renderer2D::BindLayerDepth(LayerID layer, uint32_t sampler2DIndex)
	{
		auto ins = Instance();
		ASSERT_LOG_ERROR(bool(sampler2DIndex > 0 && sampler2DIndex < MAX_TEXTURE_INDEX), "sampler2DIndex must be comprised between 1 and 15");
		for (auto& b : ins->m_layerDepthBindings)
		{
			if (b.layer == layer)
			{
				b.samplerIndex = sampler2DIndex;
				return;
			}
		}
		ins->m_layerDepthBindings.push_back({ layer, sampler2DIndex });
	}

	LayerID Renderer2D::AddOffscreenLayer(std::string name, uint32_t sampler2DIndex, Material* material)
	{
		auto ins = Instance();
		ins->m_queue.thread.Queue(&Renderer2D::AddOffscreenLayerThread, ins.get(), name, sampler2DIndex, material);
		ins->Wait();
		auto& layer = ins->m_layers[(size_t)ins->m_lastLayerAdded];
		for (auto& mat : ins->m_materials)
		{
			if (!mat->IsLayer())
				ins->m_queue.thread.Queue(&Renderer2D::CreateQuadBatchThread, ins.get(), layer, mat);
		}
		ins->Wait();
		//ins->m_layerMax++;
		return ins->m_lastLayerAdded;
	}

	void Renderer2D::SetLayerScreenSpace(LayerID layer, const std::vector<Vec2f>& screenSpace)
	{
		auto ins = Instance();
		if (screenSpace.size() != 4)
		{
			LOG_WARN("screenSpace size is not 4. Layer screen space hasn't been changed.");
			return;
		}
		if (ins->m_layers.size() <= layer)
		{
			LOG_WARN("layer id is over layer count. Layer screen space hasn't been changed.");
			return;
		}
		if (layer == 0)
		{
			LOG_WARN("Cannot change layer screen space of the screen layer. Layer screen space hasn't been changed.");
			return;
		}
		ins->m_layers[layer].vertexArray = ins->GenerateLayerVertexArray(screenSpace, layer);
	}

	uint32_t Renderer2D::GetLayerId(std::string name)
	{
		auto ins = Instance();
		uint32_t i = 0;
		for (auto& layer : ins->m_layers)
		{
			if (layer.name == name)
			{
				return i;
			}
			i++;
		}
		LOG_WARN("No render layer with the name: " + name + " default layer returned.");
		return 0;
	}

	std::vector<uint32_t> Renderer2D::GetLayers()
	{
		auto ins = Instance();
		size_t layerCount = ins->m_layers.size();
		std::vector<uint32_t> layers(layerCount - 1);
		for (size_t i = 1; i < layerCount; i++)
		{
			layers[i] = ins->m_layers[i].index;
		}
		return layers;
	}

	Material* Renderer2D::GetMaterial(MaterialID id)
	{
		return Instance()->m_materials[(size_t)id];
	}

	Material* Renderer2D::GetDefaultQuadMaterial()
	{
		return Instance()->m_defaultBatchMaterial;
	}

	Material* Renderer2D::GetDefaultLayerMaterial()
	{
		return Instance()->m_defaultLayerMaterial;
	}

	void Renderer2D::SetLayerMaterial(LayerID layer, Material* material)
	{
		auto ins = Instance();
		ins->SetShaderUniformSampler(material->GetShader(), MAX_TEXTURE_INDEX);
		ins->m_layers[layer].material = material;
	}

	void Renderer2D::SetPostProcessMaterial(Material* material, PostStage stage)
	{
		// The uTextures sampler array is configured on the render thread when the
		// material is created (CreateMaterial -> CreateQuadBatchThread), so we only
		// need to record the material here.
		auto ins = Instance();
		ins->m_postScene.clear();
		ins->m_postOverlay.clear();
		if (material)
			(stage == PostStage::Frame ? ins->m_postOverlay : ins->m_postScene).push_back(material);
	}

	void Renderer2D::AddPostProcessMaterial(Material* material, PostStage stage)
	{
		if (material)
		{
			auto ins = Instance();
			(stage == PostStage::Frame ? ins->m_postOverlay : ins->m_postScene).push_back(material);
		}
	}

	void Renderer2D::ClearPostProcessMaterials()
	{
		auto ins = Instance();
		ins->m_postScene.clear();
		ins->m_postOverlay.clear();
	}

	void Renderer2D::SetLayerExcludeFromPost(LayerID layer, bool exclude)
	{
		auto ins = Instance();
		if (layer >= (LayerID)ins->m_layers.size())
			return;
		ins->m_layers[layer].excludeFromPost = exclude;
	}

	void Renderer2D::SetLayerHeight(LayerID layer, unsigned int height)
	{
		auto ins = Instance();
		ins->m_layers[layer].height = height;
		auto windowSize = Window::GetSize();
		unsigned int width = (unsigned int)round((float)windowSize.x / (float)windowSize.y * (float)height);
		ins->m_layers[layer].target->SetSize(EvenSize(Vec2i(width, height)));
	}

	void Renderer2D::SetupMaterialThread(Material* material)
	{
		//Assign relevant texture unit to the sampler2D[] uniform uTextures
		auto shader = material->GetShader();
		SetShaderUniformSampler(shader, MAX_TEXTURE_INDEX);

		//Bind shader to the scene uniform buffer
		if (!material->IsLayer())
			shader->BindUniformBlock("scene", m_sceneUniformBinding);
	}

	void Renderer2D::CreateQuadBatchThread(RenderLayer& layer, Material* material)
	{
		if (material == nullptr)
			material = m_defaultBatchMaterial;

		//A layer / post-process material composites a fullscreen quad from its
		//OWN vertex array (GenerateLayerVertexArray for a layer, the screen
		//layer's array for a post pass) and is never a QuadRenderData::materialID,
		//so it can never receive a quad and must not be given a batch. It still
		//needs its uTextures sampler array configured on the render thread —
		//SetPostProcessMaterial documents this as the place that happens.
		if (material->IsLayer())
		{
			SetupMaterialThread(material);
			return;
		}

		LayerID layerId = layer.index;
		MaterialID matId = material->GetID();
		ASSERT_LOG_ERROR((layerId < MAX_LAYERS), "LayerId above max layer count!");
		while (m_batches[(size_t)layerId].size() <= (size_t)matId)
		{
			m_batches[(size_t)layerId].emplace_back(QuadBatch());
		}

		auto& batch = m_batches[(size_t)layerId][(size_t)matId];
		if (batch.registered)
			return;
		batch.material = material;
		batch.layer = layer;
		SetupMaterialThread(material);
		batch.registered = true;
	}

	void Renderer2D::AllocateQuadBatch(QuadBatch& batch)
	{
		//Vertex buffer
		batch.quadBuffer = makesptr<VertexBuffer>(m_maxVertices * sizeof(QuadData));
		batch.quadBuffer->SetLayout({
			{ShaderDataType::Vec3f, "iVertexPos"},
			{ShaderDataType::Vec2f, "iUv"},
			{ShaderDataType::Vec4f, "iColor"},
			{ShaderDataType::Float, "iTexIndex"},
			//Location 4. Appended AFTER iTexIndex so locations 0-3 keep their
			//meaning and every existing .vert stays valid (an undeclared
			//attribute is simply ignored). See QuadData::uvMin.
			{ShaderDataType::Vec2f, "iUvMin"}
			});

		//Vertex Array
		batch.vertexArray = makesptr<VertexArray>();
		batch.vertexArray->AddVertexBuffer(batch.quadBuffer);
		batch.vertexArray->SetIndexBuffer(m_quadIndexBuffer);

		//Vertex data on CPU
		batch.quadBase = new QuadData[m_maxVertices];

		//White texture in slot 0
		batch.textureSlots[0] = m_whiteTextureID;
	}

	void Renderer2D::ClearBatches()
	{
		auto i = Instance();
		//to do
	}
	void Renderer2D::RenderThread()
	{
		START_PROFILING("cpu_render");
		Begin();
		m_queue.Sort();
		for (int i = m_layers.size() - 1; i >= 0; i--)
		{
			auto& queue = m_queue.Get(i);
			for (int j = 0; j < queue.size(); j++)
			{
				DrawQuad(queue[j]);
			}
		}
		End();

		// The letterbox scissor is a game-scene clip; the dev ImGui overlay and
		// the software cursor overlay must draw to the full window.
		glDisable(GL_SCISSOR_TEST);

#ifdef SC_IMGUI
		if (ImGuiLoader::enabled)
		{
			ImGuiLoader::BeginImGui();
			Systems::Instance()->ImGuiUpdates();
			ImGuiLoader::EndImGui(Window::GetSize());
		}

#endif

		// Software cursor overlay (used when capture software is detected — see
		// Window::SetCursor docstring). No-op in the common case.
		Window::Instance()->RenderCursorOverlay();
		Window::RenderWindow();
		m_queue.Clear();
		STOP_PROFILING("cpu_render");
	}
	void Renderer2D::OnWindowResizeThread()
	{
		auto windowSize = Window::GetSize();
		for (auto& layer : m_layers)
		{
			if (layer.index == 0)
				continue;

			if (layer.height == 0)
			{
				//The layer fit the window size
				layer.target->SetSize(EvenSize(windowSize));
			}
			else
			{
				//The layer has it's own size, but fit the window's aspect ratio

				unsigned int width = (unsigned int)round((float)windowSize.x / (float)windowSize.y * (float)layer.height);
				layer.target->SetSize(EvenSize(Vec2i(width, layer.height)));
			}
		}
		if (m_postTarget)
			m_postTarget->SetSize(EvenSize(windowSize));
		if (m_postTargetB)
			m_postTargetB->SetSize(EvenSize(windowSize));
	}
	void Renderer2D::AddLayerThread(std::string name, unsigned int height, Material* material)
	{
		if (material == nullptr)
			material = m_defaultLayerMaterial;

		SetShaderUniformSampler(material->GetShader(), MAX_TEXTURE_INDEX);
		LayerID index = (LayerID)m_layers.size();
		std::vector<Vec2f> screenSpace{ { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		sptr<VertexArray> layerVertexArray = GenerateLayerVertexArray(screenSpace, index);
		auto windowSize = Window::GetSize();
		sptr<RenderTexture> layer;
		if (height == 0)
		{
			layer = makesptr<RenderTexture>(EvenSize(windowSize));
		}
		else
		{
			unsigned int width = (unsigned int)round((float)windowSize.x / (float)windowSize.y * (float)height);
			layer = makesptr<RenderTexture>(EvenSize(Vec2i(width, height)));
		}
		m_layers.push_back(RenderLayer(name, index, layer, material, layerVertexArray, height, false, false));
		m_renderLayers.push_back(&m_layers.back());

		m_lastLayerAdded = index;
	}
	void Renderer2D::AddOffscreenLayerThread(std::string name, uint32_t sampler2DIndex, Material* material)
	{
		if (material == nullptr)
			material = m_defaultLayerMaterial;

		ASSERT_LOG_ERROR(bool(sampler2DIndex > 0 && sampler2DIndex < 16), "sampler2DIndex must be comprised between 1 and 15");
		ASSERT_LOG_ERROR(bool(m_offscreenLayers.size() < 15), "Number of Offscreen layers exceeded (max 15)");

		LayerID index = (LayerID)m_layers.size();
		std::vector<Vec2f> screenSpace{ { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		sptr<VertexArray> layerVertexArray = GenerateLayerVertexArray(screenSpace, index);
		auto windowSize = Window::GetSize();
		sptr<RenderTexture> layer;
		/*if (height == 0)
		{*/
			layer = makesptr<RenderTexture>(EvenSize(windowSize));
		/*}
		else
		{
			unsigned int width = (unsigned int)round((float)windowSize.x / (float)windowSize.y * (float)height);
			layer = makesptr<RenderTexture>(Vec2u(width, height));
		}*/

		m_layers.push_back(RenderLayer(name, index, layer, material, layerVertexArray, 0, false, true));
		m_offscreenLayers.push_back(OffscreenRenderLayer(layer, sampler2DIndex, index));

		m_lastLayerAdded = index;
	}
	void Renderer2D::CreateSubTextureThread(const Texture* source, Rect region)
	{
		m_createdTexture = new Texture(source, region);
	}
	void Renderer2D::Begin()
	{
		//Best blending setting for normal stuff
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		auto camera = Camera::main;
		m_rendering = true;

		for (auto& layer : m_layers)
		{
			layer.target->Clear();
			layer.active = false;
		}

		if (!m_postScene.empty() || !m_postOverlay.empty())
		{
			if (!m_postTarget)
				m_postTarget = makesptr<RenderTexture>(EvenSize(Window::GetSize()));
			m_postTarget->Clear();
			SetRenderTarget(m_postTarget);
		}
		else
		{
			SetRenderTarget(Window::Instance());
		}
		//Scene data
		m_sceneUniform.camProjView = camera->GetProjectionMatrix() * camera->GetViewMatrix();
		m_sceneUniform.camZoom = camera->zoom * 2.f;
		m_sceneUniform.camAspectRatio = camera->GetAspectRatio();
		// Match the even-floored size the viewport/scissor actually use (see
		// Window::Bind / RenderTexture::Bind). Feeding the shader the raw window
		// size instead desyncs the per-fragment band math from the rasterized
		// rows and splits the bars asymmetrically on odd/scaled pixel sizes
		// (e.g. Steam Deck 1280x800 via gamescope).
		Vec2i winPx = { (int)Math::FloorToEven(Window::GetSize().x),
						(int)Math::FloorToEven(Window::GetSize().y) };
		Vec2u tgt = camera->GetTargetSize();
		auto contraints = camera->GetConstraints();
		m_sceneUniform.winSize = (Vec2f)winPx;
		m_sceneUniform.targetSize = (Vec2f)tgt;
		m_sceneUniform.reduction = camera->GetReduction();
		m_sceneUniform.cropMask = 0;
		if (contraints.cropH)
			m_sceneUniform.cropMask |= (1 << 0);
		if (contraints.cropW)
			m_sceneUniform.cropMask |= (1 << 1);
		m_sceneUniformBuffer->SetData(&m_sceneUniform, sizeof(SceneBufferData), 0);

		//Pixel-perfect snapping state for DrawQuad. With pxStep constraints the
		//world->pixel scale is a whole number, but nothing forces quad corners
		//onto whole pixels: a centered origin on an odd-sized sprite puts the
		//corners on half units, which lands on half pixels at odd scales (1x,
		//3x...) and makes nearest sampling drop/double texel rows. DrawQuad
		//translates each quad to the nearest pixel of its target FBO.
		m_pxSnap = contraints.pxStep > 0;
		if (m_pxSnap)
		{
			auto camPos = camera->GetPosition();
			m_pxSnapCam = { camPos.x, camPos.y };
			m_pxSnapZoomRed = camera->zoom * camera->GetReduction();
			m_pxSnapWinH = (float)winPx.y;
		}

		// Authoritative letterbox: a hardware scissor on the centered target
		// rect, enforced on every window write (layer composite, post-process,
		// GPU particles, custom shaders). The per-fragment crop in the quad
		// shaders is now a redundant early-out. Scissor origin is bottom-left,
		// matching gl_FragCoord, so the band math above and the rect agree.
		if (contraints.cropW || contraints.cropH)
		{
			int x = 0, y = 0, w = winPx.x, h = winPx.y;
			if (contraints.cropW && (int)tgt.x < winPx.x)
			{
				w = (int)tgt.x;
				x = (winPx.x - w) / 2;
			}
			if (contraints.cropH && (int)tgt.y < winPx.y)
			{
				h = (int)tgt.y;
				y = (winPx.y - h) / 2;
			}
			Window::SetLetterbox(x, y, w, h);
		}
		else
			Window::ClearLetterbox();

		//ResetStats
		m_stats.drawCalls = 0;
		m_stats.quadCount = 0;

		StartBatches();
	}

	void Renderer2D::End()
	{
		FlushBatches();
		RenderGpuParticles();
		RenderLayers();
		if ((!m_postScene.empty() || !m_postOverlay.empty()) && m_postTarget)
			PostProcessPass();
		m_rendering = false;
	}

	sptr<RenderTexture> Renderer2D::RunEffectChain(const std::vector<Material*>& chain, sptr<RenderTexture> src, bool endOnWindow)
	{
		//Each pass covers the full screen and fully replaces its target
		//(GL_ONE, GL_ZERO), so no per-target clear is needed. GL_BLEND stays
		//enabled with a replace func; Begin() resets the func next frame so normal
		//layer rendering is unaffected.
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);

		//Reuse the screen layer's fullscreen NDC quad to drive every pass.
		auto& screenLayer = m_layers[0];
		GLuint indicesCount = screenLayer.vertexArray->GetIndexBuffer()->GetCount();

		size_t count = chain.size();
		sptr<RenderTexture> cur = src;

		for (size_t i = 0; i < count; i++)
		{
			bool last = (i + 1 == count);
			bool toWindow = last && endOnWindow;

			sptr<RenderTexture> dst = nullptr;
			if (toWindow)
			{
				Window::Instance()->Bind();
			}
			else
			{
				if (!m_postTargetB)
					m_postTargetB = makesptr<RenderTexture>(EvenSize(Window::GetSize()));
				dst = (cur == m_postTarget) ? m_postTargetB : m_postTarget;
				dst->Bind();
			}

			cur->BindTexture(0);
			screenLayer.vertexArray->Bind();
			chain[i]->Bind();
			glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);

			if (!toWindow)
				cur = dst;
		}

		return endOnWindow ? nullptr : cur;
	}

	void Renderer2D::PostProcessPass()
	{
		//Pipeline: non-excluded layers are already composited into m_postTarget.
		//  Scene passes  -> excluded-layer (UI) composite -> Frame passes -> window.
		//Vignette-style Scene passes never touch the UI; noise-style Frame passes do.
		bool hasScene = !m_postScene.empty();
		bool hasOverlay = !m_postOverlay.empty();

		if (hasOverlay)
		{
			//Scene chain must leave its result in a buffer so we can composite the UI
			//on top before the Frame chain processes the whole thing.
			sptr<RenderTexture> cur = m_postTarget;
			if (hasScene)
				cur = RunEffectChain(m_postScene, m_postTarget, false);

			cur->Bind();
			CompositeExcludedLayers();

			RunEffectChain(m_postOverlay, cur, true);
		}
		else
		{
			//No Frame stage: Scene chain writes straight to the window, then the UI
			//composites on top, unprocessed.
			RunEffectChain(m_postScene, m_postTarget, true);
			Window::Instance()->Bind();
			CompositeExcludedLayers();
		}
	}

	void Renderer2D::BindAuxLayerTextures()
	{
		//Put offscreen layer in according texture slots
		for (auto& offscreenLayer : m_offscreenLayers)
		{
			offscreenLayer.target->BindTexture(offscreenLayer.textureUnit);
		}

		//Expose registered layer depth attachments to additional sampler units.
		for (auto& depthBind : m_layerDepthBindings)
		{
			if (depthBind.layer >= (LayerID)m_layers.size())
				continue;
			auto& src = m_layers[depthBind.layer];
			auto rt = std::dynamic_pointer_cast<RenderTexture>(src.target);
			if (!rt)
				continue;
			rt->BindDepthTexture(depthBind.samplerIndex);
		}
	}

	void Renderer2D::CompositeLayer(RenderLayer& layer)
	{
		std::static_pointer_cast<RenderTexture>(layer.target)->BindTexture(0);
		layer.vertexArray->Bind();
		layer.material->Bind();
		GLuint indicesCount = layer.vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	}

	void Renderer2D::RenderLayers()
	{
		//Premultiplied alpha for nice blending between layers
		glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA,
			GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		//Bind target framebuffer (the post target when a chain is active, else window)
		m_target->Bind();

		BindAuxLayerTextures();

		//When a post chain is active, post-excluded layers are composited later,
		//after the Scene-stage passes (see CompositeExcludedLayers / PostProcessPass).
		bool postActive = (!m_postScene.empty() || !m_postOverlay.empty()) && m_postTarget;

		//Draw every layers
		for (auto layer = m_layers.rbegin(); layer != m_layers.rend(); layer++)
		{
			if (!layer->active || layer->index == 0 || layer->offscreen)
				continue;
			if (postActive && layer->excludeFromPost)
				continue;

			CompositeLayer(*layer);
		}
	}

	void Renderer2D::CompositeExcludedLayers()
	{
		//Composite post-excluded layers (UI/HUD) onto the currently bound framebuffer,
		//on top of the Scene-processed image. Premultiplied alpha, like the normal pass.
		//The caller binds the target FB beforehand.
		glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA,
			GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		BindAuxLayerTextures();

		for (auto layer = m_layers.rbegin(); layer != m_layers.rend(); layer++)
		{
			if (!layer->active || layer->index == 0 || layer->offscreen)
				continue;
			if (!layer->excludeFromPost)
				continue;

			CompositeLayer(*layer);
		}
	}

	void Renderer2D::Flush(QuadBatch& batch)
	{
		//Send the vertex data from CPU to GPU
		uint32_t dataSize = (uint32_t)((uint8_t*)batch.quadPtr - (uint8_t*)batch.quadBase);
		if (dataSize <= 0)
			return;
		batch.quadBuffer->SetData(batch.quadBase, dataSize);

		for (uint32_t i = 0; i < batch.textureSlotIndex; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, batch.textureSlots[i]);
		}

		//Issue the draw call after binding adequat context
		batch.layer.target->Bind();
		batch.vertexArray->Bind();
		batch.material->Bind();

		glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, 0);

		m_stats.drawCalls++;
		m_layers[batch.layer.index].active = true;
	}

	Material* Renderer2D::CreateMaterial(Shader* shader, bool isLayer)
	{
		auto ins = Instance();
		Material* material = ins->m_materials.emplace_back(
			new Material(shader, (MaterialID)ins->m_materials.size(), isLayer));

		//A layer / post-process material composites its own fullscreen quad and can
		//never be a QuadRenderData::materialID, so it gets no batch on any layer —
		//only the render-thread uniform wiring it needs to be bound. Queueing a
		//batch for it used to cost one full batch PER EXISTING LAYER for nothing.
		if (isLayer)
		{
			ins->m_queue.thread.Queue(&Renderer2D::SetupMaterialThread, ins.get(), material);
			ins->Wait();
			return material;
		}

		//Register (not allocate) a quad batch on every layer
		for (auto& layer : ins->m_layers)
		{
			ins->m_queue.thread.Queue(&Renderer2D::CreateQuadBatchThread, ins.get(), layer, material);
		}
		ins->Wait();
		return material;

	}
	Texture* Renderer2D::CreateSubTexture(const Texture* source, Rect region)
	{
		auto ins = Instance();
		ins->m_queue.thread.Queue(&Renderer2D::CreateSubTextureThread, ins.get(), source, region);
		ins->Wait();
		return ins->m_createdTexture;

	}
	void Renderer2D::DrawQuad(const QuadRenderData& quad)
	{
#ifndef SANDCASTLE_DISTRIB
		//The pair must have been registered (CreateQuadBatchThread) or `batch`
		//below is a filler slot with a null material — or past the end of the
		//vector entirely. Both are UB in a distrib build; report it here instead.
		//The only ways to get here are a layer/post-process material used as a
		//quad material, or a hand-built QuadRenderData with a bogus id.
		auto& layerBatches = m_batches[(size_t)quad.layerID];
		if ((size_t)quad.materialID >= layerBatches.size()
			|| !layerBatches[(size_t)quad.materialID].registered)
		{
			LOG_ERROR("Quad pushed on layer {0} with material {1}, which has no batch there."
				" A layer/post-process material cannot draw quads. Quad dropped.",
				(uint32_t)quad.layerID, (uint32_t)quad.materialID);
			return;
		}
#endif
		auto& batch = m_batches[(size_t)quad.layerID][(size_t)quad.materialID];

		//Buffers are created on the first quad this pair ever receives, not when
		//the material is created: a batch is m_maxVertices * sizeof(QuadData) on
		//the GPU plus as much again on the CPU, and the large majority of
		//(layer, material) pairs never pair up. One predictable branch per quad
		//after the first frame. We are on the render thread, so the GL objects
		//are created on the context that owns them.
		if (!batch.allocated)
		{
			AllocateQuadBatch(batch);
			StartBatch(batch);
			batch.allocated = true;
		}

		float textureIndex = -1.0f;

		//Check if we still have space in the batch for more indices
		if (batch.indexCount >= m_maxIndices)
		{
			FlushBatches();
			StartBatches();
		}

		//Find if the texture has been used in the current batch
		for (uint32_t i = 1; i < batch.textureSlotIndex; i++)
		{
			if (batch.textureSlots[i] == quad.textureID)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex < 0.0f)
		{
			//Check if there is still space for a texture
			if (batch.textureSlotIndex >= MAX_TEXTURE_INDEX)
			{
				FlushBatches();
				StartBatches();
			}

			//Set the current texture index
			textureIndex = (float)batch.textureSlotIndex;
			//Add the texture in the appropriate slot
			batch.textureSlots[batch.textureSlotIndex] = quad.textureID;
			//Increment the current index
			batch.textureSlotIndex++;
		}

		//Input the vertex data to CPU within the quad vertex array
		static Vec2f quadVertexPosition[4]
		{
			{-0.5f, -0.5f},
			{0.5f, -0.5f},
			{0.5f, 0.5f},
			{-0.5f, 0.5f}
		};
		Vec2f origin((float)quad.orgX, (float)quad.orgY);
		QuadData* firstVertex = batch.quadPtr;
		//Frame origin for iUvMin. Only meaningful for textured quads: for
		//type == 0 `uvs` holds a colour, so feed zero instead of garbage.
		Vec2f uvMin = quad.type == 0 ? Vec2f{ 0.f, 0.f } : Vec2f{ quad.uvs.x, quad.uvs.y };
		for (int i = 0; i < 4; i++)
		{
			auto vertPos = (quadVertexPosition[i] - origin);// *m_sceneUniform.reduction;
			batch.quadPtr->vertexPos = VertexPos(vertPos, quad.pos, quad.size, quad.rotation);
			batch.quadPtr->uv = Uv(quadVertexPosition[i], quad.type, quad.uvs);
			batch.quadPtr->color = quad.color;
			batch.quadPtr->texIndex = textureIndex;
			batch.quadPtr->uvMin = uvMin;

			//Incrementing the pointed value of the quad vertex array
			batch.quadPtr++;
		}

		if (m_pxSnap && quad.rotation == 0.f)
		{
			//Mirror of the GPU mapping (default.vert + ortho projection):
			//  pxX = 0.5 * VPW - S * (worldX - camX)
			//  pxY = 0.5 * VPH + S * (worldY - camY)
			//with S = zoom * reduction * VPH, VP even-floored. The 0.5 * VP terms
			//are whole pixels, so only the S term decides the fractional part.
			//The whole quad is translated by the delta of its first corner:
			//translation-only keeps the quad's size, and since the size maps to
			//whole pixels every corner aligns.
			//Only quads that CAN be texel-aligned are snapped: rotated or
			//fractional-pixel-sized quads (spinning/scaled particles, text
			//glyphs, load bars) gain nothing and would wobble by a pixel as
			//their corner drifts across rounding boundaries while moving.
			float fboH = m_pxSnapWinH;
			if (quad.layerID < (LayerID)m_layers.size() && m_layers[quad.layerID].height != 0)
				fboH = (float)Math::FloorToEven(m_layers[quad.layerID].height);
			float pxPerUnit = m_pxSnapZoomRed * fboH;
			float pxW = quad.size.x * pxPerUnit;
			float pxH = quad.size.y * pxPerUnit;
			bool wholeSize = std::abs(pxW - std::round(pxW)) < 0.001f
				&& std::abs(pxH - std::round(pxH)) < 0.001f;
			if (pxPerUnit > 0.f && wholeSize)
			{
				float px = -pxPerUnit * (firstVertex->vertexPos.x - m_pxSnapCam.x);
				float py = pxPerUnit * (firstVertex->vertexPos.y - m_pxSnapCam.y);
				float dx = -(std::round(px) - px) / pxPerUnit;
				float dy = (std::round(py) - py) / pxPerUnit;
				for (int i = 0; i < 4; i++)
				{
					firstVertex[i].vertexPos.x += dx;
					firstVertex[i].vertexPos.y += dy;
				}
			}
		}

		batch.indexCount += 6;
		batch.quadCount++;
		m_stats.quadCount++;
	}
	inline Vec2f Lerp2f(const Vec2f& a, const Vec2f& b, const Vec2f& t)
	{
		return {
			a.x + (b.x - a.x) * t.x,
			a.y + (b.y - a.y) * t.y
		};
	}
	Vec2f Renderer2D::Uv(const Vec2f& vert, int type, const Vec4f& uvOrColor) const
	{
		// localUV = iVertexPos.xy + 0.5
		Vec2f localUV = { vert.x + 0.5f, vert.y + 0.5f };

		if (type == 0)
		{
			// Untextured quad: use localUV directly
			return localUV;
		}
		else
		{
			// Sprite: interpolate between uvMin and uvMax
			Vec2f uvMin = { uvOrColor.x, uvOrColor.y };
			Vec2f uvMax = { uvOrColor.z, uvOrColor.w };
			return Lerp2f(uvMin, uvMax, localUV);
		}
	}

	Vec3f Renderer2D::VertexPos(const Vec2f& vert, const Vec3f& pos, const Vec2f& size, float rot) const
	{
		// Scale from unit quad to sprite size
		Vec3f scaled{
			vert.x * size.x,
			vert.y * size.y,
			0.0f
		};

		// Convert degrees to radians 
		const float rad = rot * 0.01745329252f;
		const float s = std::sin(rad);
		const float c = std::cos(rad);

		// 2D rotation around Z
		Vec3f rotated{
			c * scaled.x - s * scaled.y,
			s * scaled.x + c * scaled.y,
			0.0f
		};

		// World position
		Vec3f worldPos{
			rotated.x + pos.x,
			rotated.y + pos.y,
			rotated.z + pos.z
		};

		return worldPos;
	}

	void Renderer2D::DrawLine(LineRenderer& line, Transform& transform, LayerID layer)
	{
		if (line.GetPointCount() < 2)
			return;
		m_layers[layer].target->Bind();
		m_layers[layer].active = true;
		/*m_defaultLineShader->SetUniform("aTransform", transform.GetTransformMatrix());
		m_defaultLineShader->SetUniform("uIndexCount", (float)line.GetPointCount());
		m_defaultLineShader->SetUniform("uEndCapVertices", line.GetEndCapVertices());
		m_defaultLineShader->SetUniformArray("uWidth", line.GetWidthArray(), (int)line.GetPointCount());
		m_defaultLineShader->SetUniform("uColor", line.GetColor());*/
		m_defaultLineShader->BindUniformBlock("scene", 0);
		m_defaultLineShader->Bind();

		line.Bind();
		glDrawElements(GL_LINE_STRIP_ADJACENCY, (GLsizei)line.GetPointCount() + 2, GL_UNSIGNED_INT, 0);
	}

	void Renderer2D::DrawWire(WireRender& wire, Transform& transform, LayerID layer)
	{
		if (wire.GetPointCount() < 2)
			return;
		m_layers[layer].target->Bind();
		m_layers[layer].active = true;
		/*	m_defaultWireShader->SetUniform("aTransform", transform.GetTransformMatrix());
			m_defaultWireShader->SetUniform("uColor", wire.GetColor());
			m_defaultWireShader->BindUniformBlock("camera", 0);*/
		m_defaultWireShader->Bind();

		wire.Bind();

		glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)wire.GetPointCount());
	}

	void Renderer2D::SetDefaultBatchMaterial(Material* material)
	{
		Instance()->m_defaultBatchMaterial = material;
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return Instance()->m_stats;
	}

	void Renderer2D::StartBatch(QuadBatch& batch)
	{
		//Reset vertex array data
		batch.quadPtr = batch.quadBase;

		//Reset counter
		batch.indexCount = 0;
		batch.quadCount = 0;

		//Reset texture slot index
		batch.textureSlotIndex = 1;
	}

	void Renderer2D::StartBatches()
	{
		for (int i = MAX_LAYERS - 1; i >= 0; i--)
		{
			for (auto& batch : m_batches[i])
			{
				if (batch.allocated)
					StartBatch(batch);
			}
		}
	}

	void Renderer2D::FlushBatches()
	{
		for (int i = MAX_LAYERS - 1; i >= 0; i--)
		{
			for (auto& batch : m_batches[i])
			{
				if (batch.allocated)
					Flush(batch);
			}
		}
	}

	void Renderer2D::OnWindowResize(Vec2u size)
	{
		Wait();
		m_queue.thread.Queue(&Renderer2D::OnWindowResizeThread, this);
	}

	void Renderer2D::Wait()
	{
		m_queue.thread.Wait();
	}

	void Renderer2D::Process()
	{
		if (!Window::GetRenderWhenMinimized() && Window::GetMinimized())
			return;
		if (Camera::main == nullptr)
			return;
		Wait();
		m_queue.Swap();
		//Flip every GPU pool's spawn queue while the render thread is idle, and
		//snapshot the frame delta the simulation step will integrate with.
		for (auto& sys : m_gpuSystems)
			if (sys)
				sys->SwapSpawns();
		m_gpuFrameDelta = (float)Time::Delta();
		m_queue.thread.Queue(&Renderer2D::RenderThread, this);
	}

	sptr<VertexArray> Renderer2D::GenerateLayerVertexArray(const std::vector<Vec2f>& screenSpace, LayerID layer)
	{
		//Create a vertex array for a layer

		//The screen coordinates the layer will be rendered into.
		float layerVertices[]
		{
		screenSpace[0].x, screenSpace[0].y, 0.f,  0.0, 0.0,
		screenSpace[1].x, screenSpace[1].y, 0.f,  1.0, 0.0,
		screenSpace[2].x, screenSpace[2].y, 0.f,  1.0, 1.0,
		screenSpace[3].x, screenSpace[3].y, 0.f,  0.0, 1.0,
		};

		AttributeLayout layout({
			{ ShaderDataType::Vec3f, "aPosition" },
			{ ShaderDataType::Vec2f, "aTexCoords" } });
		sptr<VertexBuffer> layerVertexBuffer = makesptr<VertexBuffer>(layerVertices, sizeof(layerVertices), layout);

		uint32_t layerIndices[]
		{
			0, 1, 2,
			2, 3, 0
		};

		sptr<IndexBuffer> layerIndexBuffer = makesptr<IndexBuffer>(layerIndices, 6, GL_STATIC_DRAW);

		return makesptr<VertexArray>(layerVertexBuffer, layerIndexBuffer);
	}

	GpuParticleSystemId Renderer2D::CreateGpuParticleSystem(const GpuParticleSystemDesc& desc)
	{
		auto ins = Instance();
		auto system = makesptr<GpuParticleSystem>(desc); // main thread: snapshots the sprite

		//Mutate the systems list only while the render thread is idle (it iterates the
		//list in RenderGpuParticles / Process), then allocate its GL objects there.
		ins->Wait();
		GpuParticleSystemId id = GpuParticleSystemInvalid;
		for (GpuParticleSystemId i = 0; i < (GpuParticleSystemId)ins->m_gpuSystems.size(); i++)
		{
			if (!ins->m_gpuSystems[i])
			{
				id = i;
				break;
			}
		}
		if (id == GpuParticleSystemInvalid)
		{
			id = (GpuParticleSystemId)ins->m_gpuSystems.size();
			ins->m_gpuSystems.push_back(nullptr);
		}
		ins->m_gpuSystems[id] = system;

		ins->m_queue.thread.Queue(&Renderer2D::CreateGpuParticleSystemThread, ins.get(), system);
		ins->Wait();
		return id;
	}

	void Renderer2D::EmitGpuParticle(GpuParticleSystemId id, const GpuParticleSpawn& spawn)
	{
		auto ins = Instance();
		if (id >= (GpuParticleSystemId)ins->m_gpuSystems.size())
			return;
		auto& sys = ins->m_gpuSystems[id];
		if (sys)
			sys->Emit(spawn);
	}

	void Renderer2D::DestroyGpuParticleSystem(GpuParticleSystemId id)
	{
		auto ins = Instance();
		if (id >= (GpuParticleSystemId)ins->m_gpuSystems.size())
			return;
		ins->Wait();
		auto sys = ins->m_gpuSystems[id];
		if (!sys)
			return;
		ins->m_gpuSystems[id] = nullptr; // stop iterating it immediately
		ins->m_queue.thread.Queue(&Renderer2D::DestroyGpuParticleSystemThread, ins.get(), sys);
		ins->Wait();
	}

	void Renderer2D::SetGpuParticleGravity(GpuParticleSystemId id, Vec2f gravity)
	{
		auto ins = Instance();
		if (id < (GpuParticleSystemId)ins->m_gpuSystems.size() && ins->m_gpuSystems[id])
			ins->m_gpuSystems[id]->SetGravity(gravity);
	}

	void Renderer2D::SetGpuParticleDrag(GpuParticleSystemId id, float drag)
	{
		auto ins = Instance();
		if (id < (GpuParticleSystemId)ins->m_gpuSystems.size() && ins->m_gpuSystems[id])
			ins->m_gpuSystems[id]->SetDrag(drag);
	}

	void Renderer2D::SetGpuParticleBlend(GpuParticleSystemId id, GpuParticleBlend blend)
	{
		auto ins = Instance();
		if (id < (GpuParticleSystemId)ins->m_gpuSystems.size() && ins->m_gpuSystems[id])
			ins->m_gpuSystems[id]->SetBlend(blend);
	}

	void Renderer2D::SetGpuParticleSizeEase(GpuParticleSystemId id, GpuParticleSizeEase ease)
	{
		auto ins = Instance();
		if (id < (GpuParticleSystemId)ins->m_gpuSystems.size() && ins->m_gpuSystems[id])
			ins->m_gpuSystems[id]->SetSizeEase(ease);
	}

	void Renderer2D::SetGpuParticleJitter(GpuParticleSystemId id, float amplitude, float frequency)
	{
		auto ins = Instance();
		if (id < (GpuParticleSystemId)ins->m_gpuSystems.size() && ins->m_gpuSystems[id])
			ins->m_gpuSystems[id]->SetJitter(amplitude, frequency);
	}

	void Renderer2D::EnsureGpuParticleShaders()
	{
		if (m_gpuShaders.ready)
			return;
		m_gpuShaders = GpuParticleSystem::CompileShaders(m_sceneUniformBinding);
	}

	void Renderer2D::CreateGpuParticleSystemThread(sptr<GpuParticleSystem> system)
	{
		EnsureGpuParticleShaders();
		system->InitGL();
	}

	void Renderer2D::DestroyGpuParticleSystemThread(sptr<GpuParticleSystem> system)
	{
		system->DestroyGL();
	}

	void Renderer2D::RenderGpuParticles()
	{
		//Pure addition: with no pool created the shaders are never compiled and this
		//returns before touching any GL state, so legacy frames are byte-identical.
		if (!m_gpuShaders.ready)
			return;

		bool any = false;
		for (auto& s : m_gpuSystems)
			if (s)
			{
				any = true;
				break;
			}
		if (!any)
			return;

		//Particles are painter's-ordered 2D: no depth test/write. Save & restore so
		//the surrounding pipeline state is left exactly as we found it.
		GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
		GLboolean depthMask = GL_TRUE;
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		for (auto& sys : m_gpuSystems)
		{
			if (!sys)
				continue;
			LayerID lid = sys->GetLayer();
			if (lid >= (LayerID)m_layers.size())
				continue;
			sys->ApplySpawns();                              // upload pending spawns
			sys->Update(m_gpuShaders, m_gpuFrameDelta);      // transform-feedback sim
			sys->Render(m_gpuShaders, m_layers[lid].target); // draw into the layer FBO
			m_layers[lid].active = true;                     // so RenderLayers composites it
		}

		if (depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		glDepthMask(depthMask);
	}
}