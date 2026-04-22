#pragma once

#include "SandCastle/Render/Beziers.h"

namespace SandCastle
{
	using ParticleEasingFn = double(*)(double);

	struct Particle
	{
		float t = 0.f;
		float speed = 1.f;
		float fade = 0.f;
		float scale = 0.f;
		Beziers trajectory;
		ParticleEasingFn easing = nullptr;
	};
}
