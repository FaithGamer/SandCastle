#include "pch.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/ECS/World.h"
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
		auto render = entity.AddComponent<SpriteRender>();
		auto transform = entity.AddComponent<Transform>();
		auto sprite = Assets::Get<Sprite>(defaultSprite).Ptr();
		render->SetSprite(sprite);
		return entity;
	}
	
	Entity Entity::CreateAnimatedSprite(String defaultAnimation, String defaultAnimStateName)
	{
		auto entity = Entity::Create();
		auto render = entity.AddComponent<SpriteRender>();
		auto transform = entity.AddComponent<Transform>();
		auto anim = entity.AddComponent<Animator>();
		anim->AddAnimation(defaultAnimStateName, defaultAnimation);
		anim->SetAnimation(defaultAnimStateName);
		return entity;
	}
}