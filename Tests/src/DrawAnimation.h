#pragma once

#include <SandCastle/SandCastle.h>

using namespace SandCastle;

void DrawAnimation()
{
	Engine::Init();

	auto entity = Entity::Create();
	auto transform = entity.AddComponent<Transform>();
	auto render = entity.AddComponent<SpriteRender>();

	//Add component animator to play sprite animations
	auto animator = entity.AddComponent<Animator>();
	//Add an animation to be played later at any time.
	animator->AddAnimation("test", "anim_test.anim");
	//Play this animation now.
	animator->SetAnimation("test");

	//Could be simplified like this:
	//auto entity = Entity::CreateAnimatedSprite("anim_test.anim", "test");

	Engine::Launch();
}