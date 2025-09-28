#pragma once
#include "SandCastle/ECS/System.h"
#include "SandCastle/Render/Animator.h"

namespace SandCastle
{
	class AnimationSystem : public System
	{
	public:
		void Update() override;
		int GetUsedMethod() override;
	};
}