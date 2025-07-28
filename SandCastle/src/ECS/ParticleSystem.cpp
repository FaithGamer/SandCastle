#include "pch.h"
#include "SandCastle/ECS/ParticleSystem.h"
#include "SandCastle/Render/Particle.h"
#include "SandCastle/Render/Renderer2D.h"
#include "SandCastle/Core/Random.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/ECS/Systems.h"



namespace SandCastle
{
	unsigned int ParticleSystem::m_particleCount(0);

	ParticleSystem::ParticleSystem() : m_freeSpot(0), m_lastParticle(0)
	{
		SetPriority(9999);
	}

	void ParticleSystem::OnStart()
	{
		ListenAddComponent<ParticleGenerator>(&ParticleSystem::OnAddParticle);
		m_freeSpot = 0;
		LOG_INFO("Particle System size, for {0} particles max: {1} ko", MAX_PARTICLES, (float)sizeof(Particle) * MAX_PARTICLES / 1000);
	}

	void ParticleSystem::OnUpdate(Time deltaTime)
	{
		//Particles generator update
		ForeachEntities<ParticleGenerator>([this, deltaTime](Entity entity, ParticleGenerator& generator) {

			if (generator.internalClock <= 0.f)
				generator.internalClock = generator.particleFrequency;
			else
				generator.internalClock += deltaTime;

			if (generator.internalClock >= generator.duration)
			{
				if (generator.loop)
				{
					generator.internalClock = 0;
					generator.instancedCount = 0;
					generator.deadCount = 0;
				}
				else
				{
					if (generator.destroyWhenOver && generator.deadCount >= generator.instancedCount)
						entity.Destroy();

					generator.emitting = false;
					return;
				}
			}

			if (!generator.emitting)
				return;

			int count = (int)floor((float)generator.internalClock / (float)generator.particleFrequency);
			for (unsigned int i = 0; i < count * generator.countByInstance - generator.instancedCount; i++)
			{
				InstanceParticle(generator, entity.GetId());
			}
			});

		auto& registry = Systems::GetMainWorld()->registry;

		//Particles Updates
		for (int i = 0; i < m_lastParticle; i++)
		{
			if (!m_particles[i].alive)
				continue;

			m_particles[i].internalClock += deltaTime;

			if (m_particles[i].internalClock >= m_particles[i].lifeTime)
			{
				//Particle death
				DestroyParticle(i, registry);
			}
			else
			{
				//Particle Update
				ParticleUpdate(i, deltaTime);
			}
		}

		for (size_t i = m_lastParticle - 1; i > 0; i--)
		{
			if (m_particles[i].alive)
				break;
			else
				m_lastParticle--;
		}
	}

	void ParticleSystem::OnRender()
	{
		auto& registry = Systems::GetMainWorld()->registry;
		auto renderer = Renderer2D::Instance();

		for (int i = 0; i < m_lastParticle; i++)
		{
			if (!m_particles[i].alive)
				continue;

			if (m_particles[i].needUpdateRenderBatch)
			{
				m_particles[i].renderBatch = Renderer2D::GetBatchId(m_particles[i].GetLayer(), m_particles[i].GetShader());
				m_particles[i].needUpdateRenderBatch = false;
			}
			auto generatorTransform = registry.try_get<Transform>(m_particles[i].generator);
			if (!generatorTransform)
				Entity(m_particles[i].generator).Destroy();
			else
			{
				Transform transform;
				transform.SetPosition(m_particles[i].position + generatorTransform->GetPosition());
				renderer->DrawSprite(transform, m_particles[i], m_particles[i].renderBatch);
			}
		}
	}

	int ParticleSystem::GetUsedMethod()
	{
		return Method::Render | Method::Update;
	}

	void ParticleSystem::OnAddParticle(ComponentSignal componentSignal)
	{
		//auto generator = Entity(componentSignal.entity).GetComponent<ParticleGenerator>();

	}

	unsigned int ParticleSystem::GetParticleCount()
	{
		return m_particleCount;
	}

	void ParticleSystem::DestroyParticle(size_t i, entt::registry& registry)
	{
		auto generator = registry.try_get<ParticleGenerator>(m_particles[i].generator);
		if (generator)
			generator->deadCount++;

		m_particles[i].alive = false;
		m_particleCount--;

		if (i < m_freeSpot)
		{
			m_freeSpot = i;
		}
	}

	void ParticleSystem::ParticleUpdate(size_t i, Time delta)
	{
		m_particles[i].position += m_particles[i].velocity * (float)delta;
	}

	void ParticleSystem::InstanceParticle(ParticleGenerator& generator, EntityId generatorId)
	{

		while (m_particles[m_freeSpot].alive)
		{
			m_freeSpot++;
			if (m_freeSpot >= MAX_PARTICLES)
			{
				LOG_WARN("Max particle count reach.");
				return;
			}
		}

		float angle = Random::Range(0.f, generator.spreadAngle);
		Vec2f direction = Math::AngleToVec(angle);

		m_particles[m_freeSpot].internalClock = 0.f;
		m_particles[m_freeSpot].position = { 0, 0, 0 };
		m_particles[m_freeSpot].velocity = { direction.x * generator.speed, direction.y * generator.speed, 0.f };
		m_particles[m_freeSpot].lifeTime = Random::Range((float)generator.particleLifeTime * 0.25f, (float)generator.particleLifeTime);
		m_particles[m_freeSpot].SetSprite(generator.sprite);
		m_particles[m_freeSpot].SetLayer(generator.layer);
		m_particles[m_freeSpot].SetShader(generator.shader);
		m_particles[m_freeSpot].generator = generatorId;
		m_particles[m_freeSpot].alive = true;
		generator.instancedCount++;

		m_particleCount++;


		m_lastParticle = m_freeSpot + 1 > m_lastParticle ? m_freeSpot + 1 : m_lastParticle;
	}
}