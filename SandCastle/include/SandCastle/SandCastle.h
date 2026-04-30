#pragma once

/// @file SandCastle.h
/// @brief Umbrella header for the SandCastle 2D game engine.
/// Including this single header pulls in every public module: Core utilities,
/// the Engine bootstrap, the 2D Renderer, the EnTT-based ECS, the Input map,
/// the Box2D physics wrapper, the immediate-mode UI, the audio backend, and
/// editor tools. Most user code only needs this one include.

#include "SandCastle/Core.h"
#include "SandCastle/Engine.h"
#include "SandCastle/Render.h"
#include "SandCastle/ECS.h"
#include "SandCastle/Input.h"
#include "SandCastle/PhysicsEngine.h"
#include "SandCastle/UIheader.h"
#include "SandCastle/Audioheader.h"
#include "SandCastle/Tools/SpriteExport.h"
