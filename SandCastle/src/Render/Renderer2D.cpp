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
#include "SandCastle/Internal/ImGuiLoader.h"

namespace SandCastle
{
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
		m_maxQuads = 100000;
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
		m_materials.emplace_back(new Material(defaultShader, (MaterialID)m_materials.size(), false));
		m_defaultBatchMaterial = m_materials.back();
		m_defaultLayerMaterial = CreateMaterial(Assets::Get<Shader>("default_layer.shader"), true);
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
		m_layerMax++;
		m_lastLayerAdded = m_layerMax;

		//Create initial quad batch for windoww layer and default batch material
		Renderer2D::CreateQuadBatchThread(m_layers.back(), m_defaultBatchMaterial);
		SetShaderUniformSampler(m_defaultLayerMaterial->GetShader(), MAX_OFF_LAYERS + 1);

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
			if(!mat->IsLayer())
				ins->m_queue.thread.Queue(&Renderer2D::CreateQuadBatchThread, ins.get(), layer, mat);
		}
		ins->Wait();
		ins->m_layerMax++;
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

	LayerID Renderer2D::AddOffscreenLayer(std::string name, uint32_t sampler2DIndex)
	{
		auto ins = Instance();
		ins->Wait();
		ASSERT_LOG_ERROR(bool(sampler2DIndex > 0 && sampler2DIndex < 16), "sampler2DIndex must be comprised between 1 and 15");
		ASSERT_LOG_ERROR(bool(ins->m_offscreenLayers.size() < 15), "Number of Offscreen layers exceeded (max 15)");

		//To do: check if sampler2DIndex hasn't been used already.

		sptr<RenderTexture> layer = makesptr<RenderTexture>(Window::GetSize());
		std::vector<Vec2f> screenSpace{ {-1, -1}, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		uint32_t index = (uint32_t)ins->m_layers.size();
		sptr<VertexArray> layerVertexArray = ins->GenerateLayerVertexArray(screenSpace, index);
		ins->m_layers.push_back(RenderLayer(name, index, layer, ins->m_defaultLayerMaterial, layerVertexArray, false, true));
		ins->m_offscreenLayers.push_back(OffscreenRenderLayer(layer, sampler2DIndex, index));

		return (uint32_t)ins->m_layers.size() - 1;
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
		ins->SetShaderUniformSampler(material->GetShader(), MAX_OFF_LAYERS + 1);
		ins->m_layers[layer].material = material;
	}

	void Renderer2D::SetLayerHeight(LayerID layer, unsigned int height)
	{
		auto ins = Instance();
		ins->m_layers[layer].height = height;
		auto windowSize = Window::GetSize();
		unsigned int width = (unsigned int)round((float)windowSize.x / (float)windowSize.y * (float)height);
		ins->m_layers[layer].target->SetSize({ width, height });
	}

	void Renderer2D::CreateQuadBatchThread(RenderLayer& layer, Material* material)
	{
		if (material == nullptr)
			material = m_defaultBatchMaterial;
		LayerID layerId = layer.index;
		MaterialID matId = material->GetID();
		ASSERT_LOG_ERROR((layerId < MAX_LAYERS), "LayerId above max layer count!");

		while (m_batches[(size_t)layerId].size() <= (size_t)matId)
		{
			m_batches[(size_t)layerId].emplace_back(QuadBatch());
		}

		auto& batch = m_batches[(size_t)layerId][(size_t)matId];
		if (batch.allocated)
			return;
		AllocateQuadBatch(batch);
		batch.material = material;
		batch.layer = layer;

		//Assign relevant texture unit to the sampler2D[] uniform uTextures
		auto shader = material->GetShader();
		SetShaderUniformSampler(shader, MAX_TEXTURE_INDEX);

		//Bind shader to the scene uniform buffer
		shader->BindUniformBlock("scene", m_sceneUniformBinding);
		batch.allocated = true;
	}

	void Renderer2D::AllocateQuadBatch(QuadBatch& batch)
	{
		//Vertex buffer
		batch.quadBuffer = makesptr<VertexBuffer>(m_maxVertices * sizeof(QuadData));
		batch.quadBuffer->SetLayout({
			{ShaderDataType::Vec3f, "iVertexPos"},
			{ShaderDataType::Vec2f, "iUv"},
			{ShaderDataType::Vec4f, "iColor"},
			{ShaderDataType::Float, "iTexIndex"}
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

#ifdef SC_IMGUI
		BeginImGui();
		Systems::Instance()->ImGuiUpdates();
		EndImGui(Window::GetSize());

#endif

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
				layer.target->SetSize((Vec2u)windowSize);
			}
			else
			{
				//The layer has it's own size, but fit the window's aspect ratio

				unsigned int width = (unsigned int)round((float)windowSize.x / (float)windowSize.y * (float)layer.height);
				layer.target->SetSize({ width, layer.height });
			}
		}
	}
	void Renderer2D::AddLayerThread(std::string name, unsigned int height, Material* material)
	{
		if (material == nullptr)
			material = m_defaultLayerMaterial;

		SetShaderUniformSampler(material->GetShader(), MAX_OFF_LAYERS + 1);
		LayerID index = (LayerID)m_layers.size();
		std::vector<Vec2f> screenSpace{ { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		sptr<VertexArray> layerVertexArray = GenerateLayerVertexArray(screenSpace, index);
		auto windowSize = Window::GetSize();
		sptr<RenderTexture> layer;
		if (height == 0)
		{
			layer = makesptr<RenderTexture>(windowSize);
		}
		else
		{
			unsigned int width = (unsigned int)round((float)windowSize.x / (float)windowSize.y * (float)height);
			layer = makesptr<RenderTexture>(Vec2u(width, height));
		}
		m_layers.push_back(RenderLayer(name, index, layer, material, layerVertexArray, height, false, false));
		m_renderLayers.push_back(&m_layers.back());

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

		SetRenderTarget(Window::Instance());
		//Scene data
		m_sceneUniform.camProjView = camera->GetProjectionMatrix() * camera->GetViewMatrix();
		m_sceneUniform.camZoom = camera->zoom * 2.f;
		m_sceneUniform.camAspectRatio = camera->GetAspectRatio();
		m_sceneUniform.winSize = (Vec2f)Window::GetSize();
		m_sceneUniform.targetSize = (Vec2f)camera->GetTargetSize();
		m_sceneUniform.reduction = camera->GetReduction();
		auto contraints = camera->GetConstraints();
		m_sceneUniform.cropMask = 0;
		if (contraints.cropH)
			m_sceneUniform.cropMask |= (1 << 0);
		if (contraints.cropW)
			m_sceneUniform.cropMask |= (1 << 1);
		m_sceneUniformBuffer->SetData(&m_sceneUniform, sizeof(SceneBufferData), 0);

		//ResetStats
		m_stats.drawCalls = 0;
		m_stats.quadCount = 0;

		StartBatches();
	}

	void Renderer2D::End()
	{
		FlushBatches();
		RenderLayers();
		m_rendering = false;
	}

	void Renderer2D::RenderLayers()
	{
		//Premultiplied alpha for nice blending between layers
		glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA,
			GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		//Bind target framebuffer
		
		m_target->Bind();

		//Put offscreen layer in according texture slots
		for (auto& offscreenLayer : m_offscreenLayers)
		{
			offscreenLayer.target->BindTexture(offscreenLayer.textureUnit);
		}

		//Draw every layers
		for (auto layer = m_layers.rbegin(); layer != m_layers.rend(); layer++)
		{
			if (!layer->active || layer->index == 0 || layer->offscreen)
				continue;

			std::static_pointer_cast<RenderTexture>(layer->target)->BindTexture(0);
			layer->vertexArray->Bind();
			layer->material->Bind();
			GLuint indicesCount = layer->vertexArray->GetIndexBuffer()->GetCount();
			glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
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
		glEnable(GL_DEPTH_TEST);
		//Issue the draw call after binding adequat context
		batch.layer.target->Bind();
		batch.vertexArray->Bind();
		batch.material->Bind();

		glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, 0);

		m_stats.drawCalls++;
		m_layers[batch.layer.index].active = true;
	}

	Material* Renderer2D::CreateMaterial(Shader* shader, bool layer)
	{
		auto ins = Instance();
		ins->m_materials.emplace_back(new Material(shader, (MaterialID)ins->m_materials.size(), layer));

		if (layer)
			return ins->m_materials.back();

		//Create quad batch for every layers
		for (auto& layer : ins->m_layers)
		{
			ins->m_queue.thread.Queue(&Renderer2D::CreateQuadBatchThread, ins.get(), layer, ins->m_materials.back());
		}
		ins->Wait();
		return ins->m_materials.back();

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
		auto& batch = m_batches[(size_t)quad.layerID][(size_t)quad.materialID];
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
		for (int i = 0; i < 4; i++)
		{
			auto vertPos = (quadVertexPosition[i] - origin);// *m_sceneUniform.reduction;
			batch.quadPtr->vertexPos = VertexPos(vertPos, quad.pos, quad.size, quad.rotation);
			batch.quadPtr->uv = Uv(quadVertexPosition[i], quad.type, quad.uvs);
			batch.quadPtr->color = quad.color;
			batch.quadPtr->texIndex = textureIndex;

			//Incrementing the pointed value of the quad vertex array
			batch.quadPtr++;
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
}