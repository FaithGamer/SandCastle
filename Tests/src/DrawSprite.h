#pragma once

#include "SandCastle/Engine.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/ECS/Systems.h"

using namespace SandCastle;

void DrawSprite()
{
	Engine::Init();

	auto entity = Entity::Create();
	auto render = entity.AddComponent<SpriteRender>();
	auto transform = entity.AddComponent<Transform>();
	auto sprite = Assets::Get<Sprite>("trollface.png_0_0").Ptr();
	render->SetSprite(sprite);
	Engine::Launch();
}

