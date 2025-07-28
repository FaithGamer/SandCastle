
#pragma once

#include "SandCastle/Engine.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/ECS/Systems.h"

using namespace SandCastle;

void DrawAnimation()
{
	Engine::Init();
	Entity::CreateAnimatedSprite();
	Engine::Launch();
}