#pragma once

#include "SandCastle/Core/Time.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/ECS/Entity.h"

namespace SandCastle
{
	class Particle : public SpriteRender
	{
	public:
		Particle();
		EntityId generator;
		Vec3f velocity = { 0, 0, 0 };
		Vec3f position = { 0, 0, 0 };
		Time internalClock;
		Time lifeTime;
		bool alive = false;
		
	};

	class ParticleGenerator 
	{
	public:
		ParticleGenerator();

		Time duration;
		Time particleLifeTime;
		Time particleFrequency;
		Time internalClock;

		unsigned int countByInstance;
		float spreadAngle;
		float speed;

		bool loop;
		bool destroyWhenOver;
		bool emitting;

		sptr<Sprite> sprite;
		uint32_t layer;
		sptr<Shader> shader;

		unsigned int instancedCount;
		unsigned int deadCount;
	private:

	};
}