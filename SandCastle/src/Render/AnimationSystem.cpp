#include "pch.h"
#include "SandCastle/Render/AnimationSystem.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Animator.h"
#include "SandCastle/Core/Assets.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{

	//Animation System
	void AnimationSystem::Update()
	{
		auto delta = Time::Delta();
		auto group = Entity::registry.group<Animator>(entt::get<SpriteRender, Transform>);
		group.each([&](Animator& animator, SpriteRender& sprite, Transform& transform)
			{
				if (animator.currentState == nullptr)
					return;

				auto anim = animator.currentState->animation;
				auto* frame = &animator.currentKeyFrame;
				float frameTime = anim->frames[*frame].timeToNext;

				while (animator.accumulator > frameTime)
				{
					//Go to next frame, or go back to frame 0, or stay and last frame
					size_t frameMax = anim->frames.size() - 1;
					(*frame)++;
					
					if (*frame > frameMax)
					{
						if (animator.currentState->transition.size() > 0)
						{
							animator.SetAnimation(animator.currentState->transition);
							anim = animator.currentState->animation;
							frame = &animator.currentKeyFrame;
						}
						else
						{
							*frame = animator.loop ? 0 : frameMax;
						}
					}
					if (anim->frames[*frame].sendSignal)
					{
						animator.currentState->signals[*frame].Send(KeyframeSignal(animator.currentStateName, *frame));
					}
					//Reset frame timer
					animator.accumulator = std::max(animator.accumulator - frameTime, 0.f);
				}
				sprite.SetSprite(anim->frames[*frame].sprite);
				animator.accumulator += (float)delta * animator.speed;
				//to do, evaluate transform
			});
	}

	int AnimationSystem::GetUsedMethod()
	{
		return System::Method::Updt;
	}

}