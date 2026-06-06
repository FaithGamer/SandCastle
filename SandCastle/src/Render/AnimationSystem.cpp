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
		float scaled = (float)Time::Delta();
		float unscaled = (float)Time::UnscaledDelta();
		auto group = Entity::View<Animator, SpriteRender, Transform>();
		group.each([&](Animator& animator, SpriteRender& sprite, Transform& transform)
			{
				if (animator.currentState == nullptr)
					return;
				float delta = animator.unscaled ? unscaled : scaled;

				auto anim = animator.currentState->animation;
				auto* frame = &animator.currentKeyFrame;
				float frameTime = anim->frames[*frame].timeToNext;

				struct Call
				{
					int frame;
					AnimationState* state;
					String name;
				};
				std::vector<Call> calls;
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
						calls.emplace_back(*frame, animator.currentState, animator.currentStateName);
					}
					//Reset frame timer
					animator.accumulator = std::max(animator.accumulator - frameTime, 0.f);
				}
				sprite.SetSprite(anim->frames[*frame].sprite);
				animator.accumulator += delta * animator.speed;

				//Despite having the callback at the end, it is NOT safe to destroy the entity within the callback
				for (auto& c : calls)
				{
					animator.animations[c.name].signals[c.frame].Send(KeyframeSignal(c.name, c.frame));
				}
				//to do, evaluate transform
			});
	}

	int AnimationSystem::GetUsedMethod()
	{
		return System::Method::Updt;
	}

}