#pragma once

#include <SandCastle/SandCastle.h>

using namespace SandCastle;

void DrawAnimation()
{
	Engine::Init();

	auto entity = Entity::Create();
	entity.Add<Transform>();
	entity.Add<SpriteRender>();
	auto transform = entity.Get<Transform>();
	auto render = entity.Get<SpriteRender>();

	//Add component animator to play sprite animations
	auto animator = entity.AddGet<Animator>();
	//Add an animation to be played later at any time.
	animator->AddAnimation("test", "anim_test.anim");
	//Play this animation now.
	animator->SetAnimation("test");

	//Could be simplified like this:
	//auto entity = Entity::CreateAnimatedSprite("anim_test.anim", "test");

	Engine::Launch();
}