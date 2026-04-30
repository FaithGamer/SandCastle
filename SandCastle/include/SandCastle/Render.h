#pragma once

/// @file Render.h
/// @brief Aggregate include for the Render module: 2D batched renderer, layers,
/// materials/shaders, sprites and animations, text writer, particles, line/wire
/// helpers, render targets, camera and window. The renderer batches quads per
/// layer/material and runs on its own thread fed by Renderer2D::PushQuad.

#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Render/Buffer.h"
#include "SandCastle/Render/Camera.h"
#include "SandCastle/Render/Shader.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/VertexArray.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Render/IndexBuffer.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/LineRenderer.h"
#include "SandCastle/Render/WireRender.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Render/RenderOptions.h"
#include "SandCastle/Render/RenderTexture.h"
#include "SandCastle/Render/AnimationSystem.h"
#include "SandCastle/Render/Writer.h"
#include "SandCastle/Render/Beziers.h"
#include "SandCastle/Render/Particle.h"
#include "SandCastle/Render/ParticleSystem.h"
