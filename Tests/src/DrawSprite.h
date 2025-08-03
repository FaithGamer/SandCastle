#pragma once

#include <SandCastle/SandCastle.h>

using namespace SandCastle;

void DrawSprite()
{
	Engine::Init();

	//Create an entity
	auto entity = Entity::Create();
	//Add a Transform component to exists in world space
	auto transform = entity.AddComponent<Transform>();
	//Add a SpriteRender component to draw a textured quad
	auto render = entity.AddComponent<SpriteRender>();
	
	//Fetch a sprite in the assets, and apply it to the SpriteRender.
	auto sprite = Assets::Get<Sprite>("trollface.png_0_0");
	render->SetSprite(sprite);

	//Can be simplified with this:
	//Entity::CreateSprite("trollface.png_0_0");

	Engine::Launch();
}

