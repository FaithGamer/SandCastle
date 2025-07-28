#include "pch.h"
#include "SandCastle/Render/Particle.h"

namespace SandCastle
{
	ParticleGenerator::ParticleGenerator() : 
		duration(2), particleLifeTime(1), particleFrequency(0.3f), countByInstance(1),
		spreadAngle(360), speed(20.f), loop(false), destroyWhenOver(true), emitting(true),
		instancedCount(0), deadCount(0), layer(0)
	{

		static auto texture = makesptr<Texture>("assets/textures/square.png", 
			TextureImportSettings(TextureFiltering::Nearest, TextureWrapping::Clamp, 16, true, false));
		static auto s = makesptr<Sprite>(texture);
		sprite = s;
	}

	Particle::Particle() : SpriteRender()
	{
		m_layer = 0;
	}
}