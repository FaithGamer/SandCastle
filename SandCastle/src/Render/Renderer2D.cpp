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
		ASSERT_LOG_ERROR(Window::IsInitialized(), "Cannot create Renderer2D before Window is initialized.");

		m_rendering = false;
		//Limitations
		m_maxInstances = 100000;
		m_maxVertices = m_maxInstances * 4;
		m_maxIndices = m_maxInstances * 6;
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
		m_cameraUniformBuffer = makesptr<UniformBuffer>(sizeof(CameraBufferData), 0);

		//Static quad vertex buffer, same for every instance
		
		/*AttributeLayout layout({
			{ ShaderDataType::Vec2f, "iVertexPos" }
			});
		m_quadVertexBuffer = makesptr<VertexBuffer>(
			(const void*)quadVertexPosition,
			sizeof(quadVertexPosition),
			layout);*/

		m_whiteTexture = new Texture();
		m_whiteTextureID = m_whiteTexture->GetId();

		m_defaultRenderOptions = makesptr<RenderOptions>();
		m_defaultRenderOptionsLayer = makesptr<RenderOptions>();
		m_defaultRenderOptionsLayer->SetDepthTest(false);
		auto window = Window::Instance();

		m_batchShader = Assets::Get<Shader>("batch_renderer.shader");
		m_defaultLayerShader = Assets::Get<Shader>("default_layer.shader");
		m_defaultLineShader = Assets::Get<Shader>("line.shader");
		m_defaultWireShader = Assets::Get<Shader>("wire.shader");

		std::vector<Vec2f> screenSpace{ {-1, -1}, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		sptr<VertexArray> defaultLayerVertexArray = GenerateLayerVertexArray(screenSpace);

		//Screen layer
		m_layers.push_back(RenderLayer("Window", 0, window, m_defaultLayerShader, m_defaultRenderOptionsLayer, defaultLayerVertexArray));
		//CreateQuadBatch(m_layers[0], nullptr, nullptr);

		SetShaderUniformSampler(m_defaultLayerShader, m_maxOffscreenLayers + 1);

		//Listen to window resize signal

		Window::GetResizeSignal()->AddListener(&Renderer2D::OnWindowResize, this);

		//Set render target to be the window by default
		SetRenderTarget(window);
	}

	Renderer2D::~Renderer2D()
	{
		//To do
		delete m_whiteTexture;
	}

	void Renderer2D::SetShaderUniformSampler(Shader* shader, uint32_t count)
	{
		std::vector<int> sampler;
		for (uint32_t i = 0; i < count; i++)
		{
			sampler.emplace_back(i);
		}
		shader->SetUniformArray("uTextures", &sampler[0], (GLsizei)sampler.size());
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

	uint32_t Renderer2D::AddLayer(std::string name, unsigned int height, Shader* shader, sptr<RenderOptions> renderOptions)
	{
		auto ins = Instance();
		if (shader == nullptr)
			shader = ins->m_defaultLayerShader;
		if (renderOptions == nullptr)
			renderOptions = ins->m_defaultRenderOptionsLayer;

		ins->SetShaderUniformSampler(shader, ins->m_maxOffscreenLayers + 1);

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
		ins->m_layers.push_back(RenderLayer(name, (uint32_t)ins->m_layers.size(), layer, shader, renderOptions, layerVertexArray, height, false, false));
		ins->m_renderLayers.push_back(&ins->m_layers.back());

		return (uint32_t)ins->m_layers.size() - 1;
	}

	uint32_t Renderer2D::AddLayer(std::string name, Shader* shader, sptr<RenderOptions> renderOptions)
	{
		return AddLayer(name, 0, shader, renderOptions);
	}

	uint32_t Renderer2D::AddOffscreenLayer(std::string name, uint32_t sampler2DIndex)
	{
		auto ins = Instance();
		ASSERT_LOG_ERROR(bool(sampler2DIndex > 0 && sampler2DIndex < 16), "sampler2DIndex must be comprised between 1 and 15");
		ASSERT_LOG_ERROR(bool(ins->m_offscreenLayers.size() < 15), "Number of Offscreen layers exceeded (max 15)");

		//To do: check if sampler2DIndex hasn't been used already.

		sptr<RenderTexture> layer = makesptr<RenderTexture>(Window::GetSize());
		std::vector<Vec2f> screenSpace{ {-1, -1}, { 1, -1 }, { 1, 1 }, { -1, 1 } };
		sptr<VertexArray> layerVertexArray = ins->GenerateLayerVertexArray(screenSpace);
		uint32_t index = (uint32_t)ins->m_layers.size();
		ins->m_layers.push_back(RenderLayer(name, index, layer, ins->m_defaultLayerShader, ins->m_defaultRenderOptionsLayer, layerVertexArray, false, true));
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

	uint32_t Renderer2D::GetBatchId(uint32_t layerIndex, Shader* shader)
	{
		//To do: bake the batchs based on assets
		auto ins = Instance();

		uint32_t shaderId = 0;
		if (shader != nullptr)
			shaderId = shader->GetID();

		uint64_t id = ins->GenerateBatchId(layerIndex, shaderId);

		auto batch = ins->m_quadBatchFinder.find(id);
		if (batch == ins->m_quadBatchFinder.end())
		{
			//Create batch if doesn't exists
			ins->CreateQuadBatch(ins->m_layers[layerIndex], shader);

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

	void Renderer2D::SetLayerShader(uint32_t layer, Shader* shader)
	{
		auto ins = Instance();
		ins->SetShaderUniformSampler(shader, ins->m_maxOffscreenLayers + 1);
		ins->m_layers[layer].shader = shader;
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

	void Renderer2D::CreateQuadBatch(RenderLayer& layer, Shader* shader)
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

		SetupQuadBatch(m_quadBatchs[index], layer, shader);
	}

	void Renderer2D::SetupQuadBatch(QuadBatch& batch, RenderLayer& layer, Shader* shader)
	{
		if (shader == nullptr)
			shader = m_batchShader;

		batch.materialShaderID = shader->GetGLID();
		batch.layer = layer;

		//Assign relevant texture unit to the sampler2D[] uniform uTextures
		std::vector<int> sampler;
		for (uint32_t i = 0; i < MAX_TEXTURE_INDEX; i++)
		{
			sampler.emplace_back(i);
		}
		shader->SetUniformArray("uTextures", &sampler[0], (GLsizei)sampler.size());

		//Bind shader to the camera uniform buffer
		shader->BindUniformBlock("camera", 0);
	}

	void Renderer2D::AllocateQuadBatch(QuadBatch& batch)
	{
		//Instance vertex buffer
		batch.instanceBuffer = makesptr<VertexBuffer>(m_maxVertices * sizeof(InstanceData));
		batch.instanceBuffer->SetLayout({
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
		//Quad buffer, same for all instances
	//	batch.vertexArray->AddVertexBuffer(m_quadVertexBuffer);
		//Instance buffer
		batch.vertexArray->AddVertexBuffer(batch.instanceBuffer);
		batch.vertexArray->SetIndexBuffer(m_quadIndexBuffer);

		//Vertex data on CPU
		batch.instanceBase = new InstanceData[m_maxVertices];

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
	void Renderer2D::Begin(const Camera& camera)
	{
		gpuTime = 0;
		m_rendering = true;

		for (auto& layer : m_layers)
		{
			layer.active = false;
		}

		//Set the camera matrices into the uniform buffer
		m_cameraUniform.projectionView = camera.GetProjectionMatrix() * camera.GetViewMatrix();
		m_cameraUniform.worldToScreenRatio = camera.worldToScreenRatio * 2;
		m_cameraUniformBuffer->SetData(&m_cameraUniform, sizeof(CameraBufferData), 0);
		//	m_defaultLineShader->SetUniform("uAspectRatio", camera.GetAspectRatio());

			//Clear layers
		for (auto& layer : m_layers)
		{
			if (layer.index == 0)
				continue;
			layer.target->Clear();
		}

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
			layer->shader->Bind();
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
		uint32_t dataSize = (uint32_t)((uint8_t*)batch.instancePtr - (uint8_t*)batch.instanceBase);
		batch.instanceBuffer->SetData(batch.instanceBase, dataSize);

		for (uint32_t i = 0; i < batch.textureSlotIndex; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, batch.textureSlots[i]);
		}

		//Issue the draw call after binding adequat context
		batch.layer.target->Bind();
		batch.vertexArray->Bind();
		glUseProgram(batch.materialShaderID);

		glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, 0);
		//glDrawElementsInstanced(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, 0, batch.instanceCount);


		m_stats.drawCalls++;
		m_layers[batch.layer.index].active = true;

		gpuTime += clock.GetElapsed();
	}

	void Renderer2D::DrawQuad(const QuadRenderData&& quad)
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
			batch.instancePtr->vertexPos = quadVertexPosition[i];
			batch.instancePtr->type = (float)quad.type;
			batch.instancePtr->pos = quad.pos;
			batch.instancePtr->uvOrColor = quad.uvOrColor;
			batch.instancePtr->size = quad.size;
			batch.instancePtr->rotation = quad.rotation;
			batch.instancePtr->texIndex = textureIndex;
			batch.instancePtr->alpha = quad.alpha;

			//Incrementing the pointed value of the quad vertex array
			batch.instancePtr++;
		}

		batch.indexCount += 6;
		batch.instanceCount++;
		m_stats.quadCount++;
	}

	void Renderer2D::DrawLine(LineRenderer& line, Transform& transform, uint32_t layer)
	{
		if (line.GetPointCount() < 2)
			return;
		m_layers[layer].target->Bind();
		m_layers[layer].active = true;
		m_defaultLineShader->SetUniform("aTransform", transform.GetTransformMatrix());
		m_defaultLineShader->SetUniform("uIndexCount", (float)line.GetPointCount());
		m_defaultLineShader->SetUniform("uEndCapVertices", line.GetEndCapVertices());
		m_defaultLineShader->SetUniformArray("uWidth", line.GetWidthArray(), (int)line.GetPointCount());
		m_defaultLineShader->SetUniform("uColor", line.GetColor());
		m_defaultLineShader->BindUniformBlock("camera", 0);
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
		m_defaultWireShader->SetUniform("aTransform", transform.GetTransformMatrix());
		m_defaultWireShader->SetUniform("uColor", wire.GetColor());
		m_defaultWireShader->BindUniformBlock("camera", 0);
		m_defaultWireShader->Bind();

		wire.Bind();

		glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)wire.GetPointCount());
	}

	void Renderer2D::SetBatchRenderShader(Shader* shader)
	{
		Instance()->m_batchShader = shader;
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return Instance()->m_stats;
	}

	void Renderer2D::StartBatch(uint32_t batchIndex)
	{
		//Reset vertex array data
		m_quadBatchs[batchIndex].instancePtr = m_quadBatchs[batchIndex].instanceBase;

		//Reset counter
		m_quadBatchs[batchIndex].indexCount = 0;
		m_quadBatchs[batchIndex].instanceCount = 0;

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