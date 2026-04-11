#include "pch.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/ECS/Systems.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/AnimationSystem.h"
#include "SandCastle/Core/Assets.h"

namespace SandCastle
{
	Entity Entity::CreateSprite(String defaultSprite)
	{
		auto entity = Entity::Create();
		entity.Add<SpriteRender>();
		entity.Add<Transform>();
		auto sprite = Assets::Get<Sprite>(defaultSprite);
		auto render = entity.Get<SpriteRender>();
		render->SetSprite(sprite);
		return entity;
	}

	Entity Entity::CreateSprite(Sprite* sprite)
	{
		auto entity = Entity::Create();
		entity.Add<SpriteRender>();
		entity.Add<Transform>();
		auto render = entity.Get<SpriteRender>();
		render->SetSprite(sprite);
		return entity;
	}
	
	Entity Entity::CreateAnimatedSprite(String defaultAnimation, String defaultAnimStateName)
	{
		auto entity = Entity::Create();
		entity.Add<SpriteRender>();
		entity.Add<Transform>();
		auto anim = entity.AddGet<Animator>();
		anim->AddAnimation(defaultAnimStateName, defaultAnimation);
		anim->SetAnimation(defaultAnimStateName);
		return entity;
	}

	Entity Entity::CreateAnimatedSprite(String animId, Animation* anim)
	{
		auto entity = Entity::Create();
		entity.Add<SpriteRender>();
		entity.Add<Transform>();
		auto anima = entity.AddGet<Animator>();
		anima->AddAnimation(animId, anim);
		anima->SetAnimation(animId);
		return entity;


		return entity;
	}
}