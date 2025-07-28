#pragma once

#include "SandCastle/ECS/System.h"
#include "SandCastle/Render/Particle.h"
#define MAX_PARTICLES 50000

namespace SandCastle
{

	class ParticleSystem : public System
	{
	public:
		ParticleSystem();
		void OnStart() override;
		void OnUpdate(Time deltaTime) override;
		void OnRender() override;
		int GetUsedMethod() override;
		void OnAddParticle(ComponentSignal componentSignal);
		static unsigned int GetParticleCount();
		size_t m_freeSpot;
		size_t m_lastParticle;

	private:
		void DestroyParticle(size_t i, entt::registry& registry);
		void ParticleUpdate(size_t i, Time delta);
		void InstanceParticle(ParticleGenerator& generator, EntityId generatorId);
		Particle m_particles[MAX_PARTICLES];


		static unsigned int m_particleCount;

	};
}
