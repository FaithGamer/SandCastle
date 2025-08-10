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
#include "SandCastle/ECS/SpriteRenderSystem.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Core/Container.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Core/Print.h"
#include "SandCastle/ECS/LineRendererSystem.h"


namespace SandCastle
{
	float Renderer2D::gpuTime = 0;
	Renderer2D::Renderer2D()
	{
		
	}

	void Renderer2D::Init()
	{
		m_thread.thread.StartThread();
		if (m_init)
		{
			LOG_ERROR("Trying to init the renderer twice.");
			return;
		}

		auto del = Delegate<void, Renderer2D>(&Renderer2D::InitThread, this);
		sptr<Task<Renderer2D>> task = makesptr<Task<Renderer2D>>(del);
		//m_thread.thread.Queue(&Renderer2D::InitThread, this);
		m_thread.thread.Queue(task);
	}

	void Renderer2D::InitThread()
	{
		if (SDL_GL_MakeCurrent(Window::GetSDLWindow(), Window::GetRenderContext()) != 0)
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
		glViewport(0, 0, size.x, size.y);

		//Enabling blending
		glEnable(GL_BLEND);

		//Enabling depth test
		glEnable(GL_DEPTH_TEST);

		//Standard blending parameters for most case uses
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		SDL_GL_SetSwapInterval(0);
	

		ASSERT_LOG_ERROR(Window::IsInitialized(), "Cannot create Renderer2D before Window is initialized.");

		m_rendering = false;
		//Limitations
		m_maxQuads = 100000;
		m_maxVertices = m_maxQuads * 4;
		m_maxIndices = m_maxQuads * 6;
		m_maxOffscreenLayers = 15;

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
		m_cameraUniformBuffer = makesptr<UniformBuffer>(sizeof(CameraBufferData), m_cameraUniformBufferBinding);

		m_whiteTexture = new Texture();
		m_whiteTextureID = m_whiteTexture->GetId();

		m_defaultRenderOptions = makesptr<RenderOptions>();
		m_defaultRenderOptionsLayer = makesptr<RenderOptions>();
		m_defaultRenderOptionsLayer->SetDepthTest(false);
		auto window = Window::Instance();

	/*m_defaultBatchMaterial = CreateMaterial(Assets::Get<Shader>("batch_renderer.shader"));
		m_defaultLayerMaterial = CreateMaterial(Assets::Get<Shader>("default_layer.shader"));
		m_defaultLineShader = Assets::Get<Shader>("line.shader");
		m_defaultWireShader = Assets::Get<Shader>("wire.shader");*/

#define LSSFF(...) Shader::LoadShaderSourceFromFile(__VA_ARGS__)

		m_defaultBatchMaterial = CreateMaterial(new Shader(LSSFF("assets/shaders/batch_renderer.vert"), LSSFF("assets/shaders/batch_renderer.frag")));
		m_defaultLayerMaterial = CreateMaterial(new Shader(LSSFF("assets/shaders/default_layer.vert"), LSSFF("assets/shaders/default_layer.frag")));
		m_defaultLineShader = new Shader(LSSFF("assets/shaders/line.vert"), LSSFF("assets/shaders/line.geom"), LSSFF("assets/shaders/line.frag"));
		m_defaultWireShader = new Shader(LSSFF("assets/shaders/wire.vert"), LSSFF("assets/shaders/wire.frag"));


		std::vector<Vec2f> screenSpace{ {-1, -1}, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		sptr<VertexArray> defaultLayerVertexArray = GenerateLayerVertexArray(screenSpace);

		//Screen layer
		m_layers.push_back(RenderLayer("Window", 0, window, m_defaultLayerMaterial, m_defaultRenderOptionsLayer, defaultLayerVertexArray));
		//CreateQuadBatch(m_layers[0], nullptr);

		SetShaderUniformSampler(m_defaultLayerMaterial->GetShader(), m_maxOffscreenLayers + 1);

		//Listen to window resize signal

		Window::GetResizeSignal()->AddListener(&Renderer2D::OnWindowResize, this);

		//Set render target to be the window by default
		SetRenderTarget(window);

		m_init = true;
	}

	Renderer2D::~Renderer2D()
	{
		m_thread.thread.StopThread();
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

	uint64_t Renderer2D::GenerateBatchId(uint64_t a, uint64_t b)
	{
		b = (b + 1) * 1000;
		a = (a + 1) * 1000000;
		return a + b;
	}

	void Renderer2D::SetRenderTarget(sptr<RenderTarget> target)
	{
		m_target = target;
	}

	uint32_t Renderer2D::AddLayer(std::string name, unsigned int height, Material* material, sptr<RenderOptions> renderOptions)
	{
		auto ins = Instance();
		ins->Wait();

		ASSERT_LOG_ERROR((ins->m_layers.size() < (MAX_LAYERS / 2)), "Max render layer ({0}) reached.", MAX_LAYERS / 2);

		if (material == nullptr)
			material = ins->m_defaultLayerMaterial;
		if (renderOptions == nullptr)
			renderOptions = ins->m_defaultRenderOptionsLayer;

		ins->SetShaderUniformSampler(material->GetShader(), ins->m_maxOffscreenLayers + 1);

		std::vector<Vec2f> screenSpace{ { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		sptr<VertexArray> layerVertexArray = ins->GenerateLayerVertexArray(screenSpace);
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
		ins->m_layers.push_back(RenderLayer(name, (uint32_t)ins->m_layers.size(), layer, material, renderOptions, layerVertexArray, height, false, false));
		ins->m_renderLayers.push_back(&ins->m_layers.back());

		return (uint32_t)ins->m_layers.size() - 1;
	}

	uint32_t Renderer2D::AddLayer(std::string name, Material* material, sptr<RenderOptions> renderOptions)
	{
		return AddLayer(name, 0, material, renderOptions);
	}

	uint32_t Renderer2D::AddOffscreenLayer(std::string name, uint32_t sampler2DIndex)
	{
		auto ins = Instance();
		ins->Wait();
		ASSERT_LOG_ERROR(bool(sampler2DIndex > 0 && sampler2DIndex < 16), "sampler2DIndex must be comprised between 1 and 15");
		ASSERT_LOG_ERROR(bool(ins->m_offscreenLayers.size() < 15), "Number of Offscreen layers exceeded (max 15)");

		//To do: check if sampler2DIndex hasn't been used already.

		sptr<RenderTexture> layer = makesptr<RenderTexture>(Window::GetSize());
		std::vector<Vec2f> screenSpace{ {-1, -1}, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		sptr<VertexArray> layerVertexArray = ins->GenerateLayerVertexArray(screenSpace);
		uint32_t index = (uint32_t)ins->m_layers.size();
		ins->m_layers.push_back(RenderLayer(name, index, layer, ins->m_defaultLayerMaterial, ins->m_defaultRenderOptionsLayer, layerVertexArray, false, true));
		ins->m_offscreenLayers.push_back(OffscreenRenderLayer(layer, sampler2DIndex, index));

		return (uint32_t)ins->m_layers.size() - 1;
	}

	void Renderer2D::SetLayerScreenSpace(uint32_t layer, const std::vector<Vec2f>& screenSpace)
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
		ins->m_layers[layer].vertexArray = ins->GenerateLayerVertexArray(screenSpace);
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

	uint32_t Renderer2D::GetBatchId(uint32_t layerIndex, Material* material)
	{
		//To do: bake the batchs based on assets
		auto ins = Instance();

		uint32_t materialId = 0;
		if (material != nullptr)
			materialId = material->GetID();

		uint64_t id = ins->GenerateBatchId(layerIndex, materialId);


		auto batch = ins->m_quadBatchFinder.find(id);
		if (batch == ins->m_quadBatchFinder.end())
		{
			//Create batch if doesn't exists
			auto del = Delegate(&Renderer2D::CreateQuadBatch, ins.get(), ins->m_layers[layerIndex], material);
			auto task = makesptr<Task<Renderer2D, RenderLayer&, Material*>>(del);
			ins->m_thread.thread.Queue(task);
			//ins->m_thread.thread.Queue(&Renderer2D::CreateQuadBatch, ins.get(), ins->m_layers[layerIndex], material);
			ins->Wait();
			//ins->CreateQuadBatch(ins->m_layers[layerIndex], material);
			uint32_t index = (uint32_t)ins->m_quadBatchs.size() - 1;
			ins->m_quadBatchs.back().index = index;

			ins->m_quadBatchFinder.insert(std::make_pair(id, index));

			if (ins->m_rendering)
			{
				ins->StartBatch(index);
			}

			return index;
		}
		else
		{
			uint32_t index = batch->second;
			return index;
		}
	}

	void Renderer2D::SetLayerMaterial(uint32_t layer, Material* material)
	{
		auto ins = Instance();
		ins->SetShaderUniformSampler(material->GetShader(), ins->m_maxOffscreenLayers + 1);
		ins->m_layers[layer].material = material;
	}

	void Renderer2D::SetLayerRenderOptions(uint32_t layer, sptr<RenderOptions> renderOptions)
	{
		auto ins = Instance();
		ins->m_layers[layer].renderOptions = renderOptions;
	}

	void Renderer2D::SetLayerHeight(uint32_t layer, unsigned int height)
	{
		auto ins = Instance();
		ins->m_layers[layer].height = height;
		auto windowSize = Window::GetSize();
		unsigned int width = (unsigned int)round((float)windowSize.x / (float)windowSize.y * (float)height);
		ins->m_layers[layer].target->SetSize({ width, height });
	}

	void Renderer2D::PreallocateQuadBatch(int count)
	{
		for (int i = 0; i < count; i++)
		{
			m_quadBatchs.push_back(QuadBatch());
			m_freeQuadBatchs.push_back(m_quadBatchs.size() - 1);
		}
	}

	void Renderer2D::CreateQuadBatch(RenderLayer& layer, Material* material)
	{
		size_t index = 0;
		if (!m_freeQuadBatchs.empty())
		{
			//Recycle a free batch
			index = m_freeQuadBatchs[0];
			Container::RemoveAt(m_freeQuadBatchs, 0);
		}
		else
		{
			//Allocate a new batch
			m_quadBatchs.push_back(QuadBatch());
			AllocateQuadBatch(m_quadBatchs.back());
			index = m_quadBatchs.size() - 1;
		}

		SetupQuadBatch(m_quadBatchs[index], layer, material);
	}

	void Renderer2D::SetupQuadBatch(QuadBatch& batch, RenderLayer& layer, Material* material)
	{
		if (material == nullptr)
			material = m_defaultBatchMaterial;

		batch.material = material;
		batch.layer = layer;

		//Assign relevant texture unit to the sampler2D[] uniform uTextures
		auto shader = material->GetShader();
		SetShaderUniformSampler(shader, MAX_TEXTURE_INDEX);

		//Bind shader to the camera uniform buffer
		shader->BindUniformBlock("camera", m_cameraUniformBufferBinding);
	}

	void Renderer2D::AllocateQuadBatch(QuadBatch& batch)
	{
		//Vertex buffer
		batch.quadBuffer = makesptr<VertexBuffer>(m_maxVertices * sizeof(InstanceData));
		batch.quadBuffer->SetLayout({
			{ShaderDataType::Vec2f, "iVertexPos"},
			{ShaderDataType::Float, "iType"},
			{ShaderDataType::Vec3f, "iPos"},
			{ShaderDataType::Vec4f, "iUvOrColor"},
			{ShaderDataType::Vec2f, "iSize"},
			{ShaderDataType::Float, "iRotation"},
			{ShaderDataType::Float, "iTexIndex"},
			{ShaderDataType::Float, "iAlpha"}
			});

		//Vertex Array
		batch.vertexArray = makesptr<VertexArray>();
		batch.vertexArray->AddVertexBuffer(batch.quadBuffer);
		batch.vertexArray->SetIndexBuffer(m_quadIndexBuffer);

		//Vertex data on CPU
		batch.quadBase = new InstanceData[m_maxVertices];

		//White texture in slot 0
		batch.textureSlots[0] = m_whiteTextureID;
	}

	void Renderer2D::FreeQuadBatch(uint32_t batch)
	{
		m_freeQuadBatchs.push_back(batch);
	}

	void Renderer2D::ClearBatches()
	{
		auto i = Instance();
		i->m_freeQuadBatchs.clear();
		i->m_quadBatchs.clear();
		i->m_quadBatchFinder.clear();
		//to do, make this a signal
		Systems::Get<SpriteRenderSystem>()->OnClearBatches();
	}
	void Renderer2D::RenderThread()
	{
		Begin();
		for (int i = 0; i < m_thread.queue[m_thread.current].size(); i++)
		{
			DrawQuad(m_thread.queue[m_thread.current][i]);
		}
		/*for (int i = m_thread.layerMax[m_thread.current]; i >= 0; i--)
		{
			for (int j = 0; j < m_thread.sorted[m_thread.current][i].size(); j++)
			{
				DrawQuad(m_thread.sorted[m_thread.current][i][j]);
			}
		}*/
		End();
		Window::RenderWindow();
		m_thread.queue[m_thread.current].clear();
		

	}
	void Renderer2D::Begin()
	{
		if (SDL_GL_GetCurrentContext() != Window::GetRenderContext())
		{
			LOG_ERROR("wrong context");
		}
		//Window::ClearWindow();
		auto camera = Systems::GetMainCamera();
		gpuTime = 0;
		m_rendering = true;

		for (auto& layer : m_layers)
		{
			layer.target->Clear();
			layer.active = false;
		}

		SetRenderTarget(Window::Instance());
		//Set the camera matrices into the uniform buffer
		m_cameraUniform.projectionView = camera->GetProjectionMatrix() * camera->GetViewMatrix();
		m_cameraUniform.worldToScreenRatio = camera->worldToScreenRatio * 2;
		m_cameraUniformBuffer->SetData(&m_cameraUniform, sizeof(CameraBufferData), 0);
		//	m_defaultLineShader->SetUniform("uAspectRatio", camera.GetAspectRatio());

		//ResetStats
		m_stats.drawCalls = 0;
		m_stats.quadCount = 0;

		for (auto& batch : m_quadBatchs)
		{
			StartBatch(batch.index);
		}
	}

	void Renderer2D::End()
	{
		for (auto& batch : m_quadBatchs)
			Flush(batch.index);
		Systems::Get<LineRendererSystem>()->Render();
		RenderLayers();
		m_rendering = false;
	}

	void Renderer2D::RenderLayers()
	{
		//Bind target framebuffer
		m_target->Bind();
		glDisable(GL_DEPTH_TEST);

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
			layer->renderOptions->Bind();
			GLuint indicesCount = layer->vertexArray->GetIndexBuffer()->GetCount();
			glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
		}
	}

	void Renderer2D::Flush(uint32_t batchIndex)
	{
		Clock clock;
		QuadBatch& batch = m_quadBatchs[(size_t)batchIndex];
		if (batch.indexCount == 0)
			return;

		//Send the vertex data from CPU to GPU
		uint32_t dataSize = (uint32_t)((uint8_t*)batch.quadPtr - (uint8_t*)batch.quadBase);
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
		//glDrawElementsInstanced(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, 0, batch.quadCount);


		m_stats.drawCalls++;
		m_layers[batch.layer.index].active = true;

		gpuTime += clock.GetElapsed();
	}

	Material* Renderer2D::CreateMaterial(Shader* shader)
	{
		m_materials.emplace_back(new Material(shader, (MaterialID)m_materials.size()));
		return m_materials.back();
	}
	void Renderer2D::PushQuad(const QuadRenderData&& quad)
	{
		int current = m_thread.current == 1 ? 0 : 1;
		//m_thread.sorted[current][quad.layerID].emplace_back(quad);
		//m_thread.layerMax[current] = quad.layerID > m_thread.layerMax[current] ? quad.layerID : m_thread.layerMax[current];
		m_thread.queue[current].emplace_back(quad);
	}
	void Renderer2D::DrawQuad(const QuadRenderData& quad)
	{
		auto& batch = m_quadBatchs[quad.batchID];
		float textureIndex = -1.0f;

		//Check if we still have space in the batch for more indices
		if (batch.indexCount >= m_maxIndices)
			NextBatch(quad.batchID);

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
				NextBatch(quad.batchID);

			//Set the current texture index
			textureIndex = (float)batch.textureSlotIndex;
			//Add the texture in the appropriate slot
			batch.textureSlots[batch.textureSlotIndex] = quad.textureID;
			//Increment the current index
			batch.textureSlotIndex++;
		}

		//bool reComputePosition = transform.matrixUpdated || transform.needCompute || spriteRender.spriteDimensionsChanged;

		//Input the vertex data to CPU within the quad vertex array
		static Vec2f quadVertexPosition[4]
		{
			{-0.5f, -0.5f},
			{0.5f, -0.5f},
			{0.5f, 0.5f},
			{-0.5f, 0.5f}
		};
		for (int i = 0; i < 4; i++)
		{
			batch.quadPtr->vertexPos = quadVertexPosition[i];
			batch.quadPtr->type = (float)quad.type;
			batch.quadPtr->pos = quad.pos;
			batch.quadPtr->uvOrColor = quad.uvOrColor;
			batch.quadPtr->size = quad.size;
			batch.quadPtr->rotation = quad.rotation;
			batch.quadPtr->texIndex = textureIndex;
			batch.quadPtr->alpha = quad.alpha;

			//Incrementing the pointed value of the quad vertex array
			batch.quadPtr++;
		}

		batch.indexCount += 6;
		batch.quadCount++;
		m_stats.quadCount++;
	}

	void Renderer2D::DrawLine(LineRenderer& line, Transform& transform, uint32_t layer)
	{
		if (line.GetPointCount() < 2)
			return;
		m_layers[layer].target->Bind();
		m_layers[layer].active = true;
		/*m_defaultLineShader->SetUniform("aTransform", transform.GetTransformMatrix());
		m_defaultLineShader->SetUniform("uIndexCount", (float)line.GetPointCount());
		m_defaultLineShader->SetUniform("uEndCapVertices", line.GetEndCapVertices());
		m_defaultLineShader->SetUniformArray("uWidth", line.GetWidthArray(), (int)line.GetPointCount());
		m_defaultLineShader->SetUniform("uColor", line.GetColor());
		m_defaultLineShader->BindUniformBlock("camera", 0);*/
		m_defaultLineShader->Bind();

		line.Bind();
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_LINE_STRIP_ADJACENCY, (GLsizei)line.GetPointCount() + 2, GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, line.GetPointCount()*2);
	}

	void Renderer2D::DrawWire(WireRender& wire, Transform& transform, uint32_t layer)
	{
		if (wire.GetPointCount() < 2)
			return;
		m_layers[layer].target->Bind();
		m_layers[layer].active = true;
		/*m_defaultWireShader->SetUniform("aTransform", transform.GetTransformMatrix());
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

	void Renderer2D::StartBatch(uint32_t batchIndex)
	{
		//Reset vertex array data
		m_quadBatchs[batchIndex].quadPtr = m_quadBatchs[batchIndex].quadBase;

		//Reset counter
		m_quadBatchs[batchIndex].indexCount = 0;
		m_quadBatchs[batchIndex].quadCount = 0;

		//Reset texture slot index
		m_quadBatchs[batchIndex].textureSlotIndex = 1;
	}

	void Renderer2D::NextBatch(uint32_t batchIndex)
	{
		Flush(batchIndex);
		StartBatch(batchIndex);
	}

	void Renderer2D::OnWindowResize(Vec2u size)
	{
		for (auto& layer : m_layers)
		{
			if (layer.index == 0)
				continue;

			if (layer.height == 0)
			{
				//The layer fit the window size
				layer.target->SetSize(size);
			}
			else
			{
				//The layer has it's own size, but fit the window's aspect ratio
				auto windowSize = Window::GetSize();
				unsigned int width = (unsigned int)round((float)windowSize.x / (float)windowSize.y * (float)layer.height);
				layer.target->SetSize({ width, layer.height });
			}
		}
	}

	void Renderer2D::Wait()
	{
		m_thread.thread.Wait();
	}

	void Renderer2D::Process()
	{
		Wait();
		m_thread.current = m_thread.current == 1 ? 0 : 1;
		auto del = Delegate<void, Renderer2D>(&Renderer2D::RenderThread, this);
		sptr<Task<Renderer2D>> task = makesptr<Task<Renderer2D>>(del);
		m_thread.thread.Queue(task);
	}

	sptr<VertexArray> Renderer2D::GenerateLayerVertexArray(const std::vector<Vec2f>& screenSpace)
	{
		//Create a vertex array for a layer

		//The screen coordinates the layer will be rendered into.
		float layerVertices[]
		{
		screenSpace[0].x, screenSpace[0].y, 0,  0.0, 0.0,
		screenSpace[1].x, screenSpace[1].y, 0,  1.0, 0.0,
		screenSpace[2].x, screenSpace[2].y, 0,  1.0, 1.0,
		screenSpace[3].x, screenSpace[3].y, 0,  0.0, 1.0,
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