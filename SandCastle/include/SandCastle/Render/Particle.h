#pragma once

#include "SandCastle/Render/Beziers.h"

namespace SandCastle
{
	/// @brief Easing function pointer used to shape particle interpolation. See Easing namespace.
	using ParticleEasingFn = double(*)(double);

	/// @brief ECS component holding per-particle state used by ParticleSystem.
	/// `t` walks from 0 to 1 along `trajectory`, modulated by `easing`. `fade`
	/// fades out alpha over the lifetime, `scale` linearly grows/shrinks size.
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
