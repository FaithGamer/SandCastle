#pragma once
#include "SandCastle/ECS/System.h"
#include "SandCastle/Render/Animator.h"

namespace SandCastle
{
	/// @brief Engine system that advances every Animator each frame and swaps the
	/// SpriteRender's sprite to the current keyframe. Push it via Systems::Push<AnimationSystem>().
	class AnimationSystem : public System
	{
	public:
		void Update() override;
		int GetUsedMethod() override;
	};
}