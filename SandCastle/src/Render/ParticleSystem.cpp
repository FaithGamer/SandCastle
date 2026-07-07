#include "pch.h"
#include <cmath>
#include <limits>
#include "SandCastle/Render/ParticleSystem.h"
#include "SandCastle/Render/Particle.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Core/Easing.h"
#include "SandCastle/Core/Random.h"
#include "SandCastle/Core/Time.h"

namespace SandCastle
{
	namespace
	{
		inline float RandRange(float a, float b)
		{
			if (a > b) std::swap(a, b);
			if (a == b) return a;
			return Random::Range(a, b);
		}

		inline int RandRange(int a, int b)
		{
			if (a > b) std::swap(a, b);
			if (a == b) return a;
			return Random::Range(a, b);
		}

		inline Color LerpColor(Color a, Color b, float t)
		{
			if (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a)
				return a;
			float u = 1.f - t;
			return Color(
				(unsigned char)(a.r * u + b.r * t),
				(unsigned char)(a.g * u + b.g * t),
				(unsigned char)(a.b * u + b.b * t),
				(unsigned char)(a.a * u + b.a * t));
		}
	}

	void ParticleSystem::Start()
	{
	}

	int ParticleSystem::GetUsedMethod()
	{
		return System::Method::Updt;
	}

	void ParticleSystem::Update()
	{
		float delta = (float)Time::Delta();

		// Drive emitters first so newly spawned particles get stepped this same frame too.
		if (m_on)
			UpdateEmitters(delta);

		auto view = Entity::View<Particle, Transform, SpriteRender>();
		int count = 0;

		view.each([&](Entity e, Particle& p, Transform& tr, SpriteRender& spr)
			{
				p.t += delta * p.speed;
				float t = p.t;
				if (p.easing != nullptr)
					t = (float)p.easing((double)p.t);

				if (p.fade > 0.01f)
				{
					float ta = Math::Clamp01((1.f - p.t) / p.fade);
					spr.color.a = (unsigned char)(255.f * ta);
				}
				else if (p.fade < -0.01f)
				{
					float ta = Math::Clamp01(p.t / std::abs(p.fade));
					spr.color.a = (unsigned char)(255.f * ta);
				}

				if (p.scale > 0.01f)
				{
					float scale = 1.f;
					if (p.t < 0.5f)
					{
						float st = p.t / 0.5f;
						scale = Math::Lerp(1.f, p.scale, (float)Easing::QuadInOut(st));
					}
					else
					{
						float st = (p.t - 0.5f) / 0.5f;
						scale = Math::Lerp(p.scale, 1.f, (float)Easing::QuadInOut(st));
					}
					tr.SetScale(scale, scale);
				}

				if (p.rotSpeed != 0.f)
				{
					p.rot += p.rotSpeed * delta;
					tr.SetRotation(p.rot);
				}

				Vec2f pos = p.trajectory.Step(t);
				float z = tr.GetPosition().z;
				tr.SetPosition(pos.x, pos.y, z);

				if (p.t >= 1.f)
				{
					if (p.destroyCb != 0)
					{
						auto it = m_destroyCallbacks.find(p.destroyCb);
						if (it != m_destroyCallbacks.end())
						{
							if (it->second.fn)
								it->second.fn(ParticleSignal{ Vec3f(pos.x, pos.y, z) });
							if (--it->second.refCount <= 0)
								m_destroyCallbacks.erase(it);
						}
					}
					e.Destroy();
					return;
				}
				count++;
			});
		m_count = count;
	}

	void ParticleSystem::Activate(bool on)
	{
		m_on = on;
		if (!on)
		{
			Entity::DestroyAll<Particle>();
			// Hard reset: particles were dropped without running their destroy
			// callbacks, so wipe the table and clear cached ids too.
			m_destroyCallbacks.clear();
			// Reset emitter timers so they don't burst-catch-up on re-activation.
			auto view = Entity::View<ParticleEmitter>();
			view.each([](ParticleEmitter& em)
				{
					em._accum = 0.f;
					em._nextInterval = -1.f;
					em._callbackId = 0;
				});
		}
	}

	Entity ParticleSystem::Make(Vec3f p1, Vec3f p2, float speed, Color color,
		ParticleTraj traj, float curveIntensity, float fade,
		double(*easing)(double), float scale)
	{
		return MakeWithSprite(m_defaultSprite, p1, p2, speed, color, traj,
			curveIntensity, fade, easing, scale);
	}

	Entity ParticleSystem::MakeWithSprite(Sprite* sprite, Vec3f p1, Vec3f p2,
		float speed, Color color, ParticleTraj traj, float curveIntensity,
		float fade, double(*easing)(double), float scale)
	{
		if (!m_on || m_count >= m_limit || sprite == nullptr)
			return Entity();

		auto entt = Entity::Create();
		entt.Add<Transform>();
		entt.Add<SpriteRender>();
		auto prt = entt.AddGet<Particle>();
		auto tr = entt.Get<Transform>();
		auto spr = entt.Get<SpriteRender>();

		spr->SetSprite(sprite);
		spr->color = color;
		tr->SetPosition(p1);
		prt->easing = easing;
		prt->speed = speed;
		prt->fade = fade;
		prt->scale = scale;

		Vec2f a(p1.x, p1.y);
		Vec2f b(p2.x, p2.y);
		switch (traj)
		{
		case ParticleTraj::Straight:
			prt->trajectory = Beziers::Straight(a, b);
			break;
		case ParticleTraj::CubicIn:
			prt->trajectory = Beziers::CubicIn(a, b, curveIntensity);
			break;
		case ParticleTraj::CubicOut:
			prt->trajectory = Beziers::CubicOut(a, b, curveIntensity);
			break;
		case ParticleTraj::CubicInOut:
			prt->trajectory = Beziers::CubicInOut(a, b, curveIntensity);
			break;
		}
		m_count++;
		return entt;
	}

	// ============================================================
	//   Emitter API
	// ============================================================

	EmitterHandle ParticleSystem::MakeEmitter(Vec3f position)
	{
		auto e = Entity::Create();
		e.Add<Transform>();
		if (auto tr = e.Get<Transform>())
			tr->SetPosition(position);
		e.Add<ParticleEmitter>();
		return EmitterHandle{ e };
	}

	EmitterHandle ParticleSystem::MakeEmitter(Entity entity)
	{
		if (!entity.Valid())
			return EmitterHandle{};
		// Transform is required for emitter position. Add a default one if absent.
		if (entity.Get<Transform>() == nullptr)
			entity.Add<Transform>();
		entity.Add<ParticleEmitter>();
		return EmitterHandle{ entity };
	}

	std::uint32_t ParticleSystem::RegisterDestroyCallback(const std::function<void(const ParticleSignal&)>& fn)
	{
		if (!fn) return 0;
		std::uint32_t id = m_nextCallbackId++;
		if (m_nextCallbackId == 0) m_nextCallbackId = 1; // skip the 0 sentinel on wrap
		m_destroyCallbacks[id].fn = fn;
		return id;
	}

	void ParticleSystem::ReleaseDestroyCallback(std::uint32_t id)
	{
		if (id == 0) return;
		auto it = m_destroyCallbacks.find(id);
		if (it != m_destroyCallbacks.end() && --it->second.refCount <= 0)
			m_destroyCallbacks.erase(it);
	}

	int ParticleSystem::Burst(Vec3f position, const ParticleEmitter& spec)
	{
		// Temporary-spec path: no persistent emitter, so register the callback (if any)
		// for just this burst. Particles ref-count it; if none spawn, drop it again.
		std::uint32_t cid = RegisterDestroyCallback(spec.onParticleDestroy);
		int spawned = Burst(position, spec, cid);
		if (cid != 0 && spawned == 0)
			m_destroyCallbacks.erase(cid);
		return spawned;
	}

	int ParticleSystem::Burst(Vec3f position, const ParticleEmitter& spec, std::uint32_t cid)
	{
		if (!m_on) return 0;
		int count = RandRange(spec.countMin, spec.countMax);
		if (count <= 0) return 0;
		int spawned = 0;
		for (int i = 0; i < count; ++i)
		{
			if (m_count >= m_limit) break;
			if (SpawnFromEmitter(position, spec, cid))
				++spawned;
		}
		return spawned;
	}

	bool ParticleSystem::SpawnFromEmitter(Vec3f origin, const ParticleEmitter& spec, std::uint32_t cid)
	{
		Sprite* sprite = spec.sprite != nullptr ? spec.sprite : m_defaultSprite;
		if (!spec.spriteChoices.empty())
			sprite = spec.spriteChoices[RandRange(0, (int)spec.spriteChoices.size() - 1)];
		if (sprite == nullptr)
			return false;

		float lifetime = RandRange(spec.lifetimeMin, spec.lifetimeMax);
		if (lifetime < 0.0001f) lifetime = 0.0001f;
		float speed = 1.f / lifetime;

		float angle = RandRange(spec.angleMin, spec.angleMax);
		float distance = RandRange(spec.distanceMin, spec.distanceMax);
		Vec2f dir = Math::AngleToVec(angle);

		// Per-particle spawn point: offset by a uniform random point inside the
		// spawn-area box (relative to the emitter origin). A zero-size box keeps
		// every particle exactly at the origin.
		Vec3f base = origin;
		const Rect& sa = spec.spawnArea;
		if (sa.width != 0.f || sa.height != 0.f)
		{
			base.x += RandRange(sa.left, sa.left + sa.width);
			base.y += RandRange(sa.Bottom(), sa.top);
		}

		Vec3f p1 = base;
		Vec3f p2(base.x + dir.x * distance, base.y + dir.y * distance, base.z);

		float curve;
		if (!spec.curveChoices.empty())
		{
			curve = Random::Pick(spec.curveChoices);
		}
		else
		{
			curve = RandRange(spec.curveMin, spec.curveMax);
			if (spec.curveBothSides && Random::Range(0.f, 1.f) < 0.5f)
				curve = -curve;
		}

		float scale = RandRange(spec.scaleMin, spec.scaleMax);

		Color color;
		if (!spec.colorChoices.empty())
		{
			// Weighted pick. Walk the list once to total the weights, then again to find
			// the slot the random draw lands in. Non-positive weights are skipped so a
			// 0-weight entry effectively disables that color without removing it.
			float total = 0.f;
			for (const auto& wc : spec.colorChoices)
				if (wc.weight > 0.f) total += wc.weight;

			if (total <= 0.f)
			{
				color = spec.colorChoices.front().color;
			}
			else
			{
				float pick = Random::Range(0.f, total);
				float acc = 0.f;
				color = spec.colorChoices.back().color;
				for (const auto& wc : spec.colorChoices)
				{
					if (wc.weight <= 0.f) continue;
					acc += wc.weight;
					if (pick <= acc) { color = wc.color; break; }
				}
			}
		}
		else
		{
			color = LerpColor(spec.colorA, spec.colorB, Random::Range(0.f, 1.f));
		}

		Entity e = MakeWithSprite(sprite, p1, p2, speed, color, spec.traj,
			curve, spec.fade, spec.easing, scale);
		if (!e.Valid())
			return false;

		// Spin: per-particle rate plus a random initial angle so a burst doesn't
		// start phase-locked. Applied every frame in Update.
		float spin = RandRange(spec.spinMin, spec.spinMax);
		if (spec.spinBothSides && Random::Range(0.f, 1.f) < 0.5f)
			spin = -spin;
		if (spin != 0.f)
		{
			if (auto prt = e.Get<Particle>())
			{
				prt->rotSpeed = spin;
				prt->rot = Random::Range(0.f, 360.f);
				if (auto tr = e.Get<Transform>())
					tr->SetRotation(prt->rot);
			}
		}

		if (cid != 0)
		{
			if (auto prt = e.Get<Particle>())
			{
				prt->destroyCb = cid;
				++m_destroyCallbacks[cid].refCount;
			}
		}

		if (spec.layer != ParticleEmitter::kKeepLayer || spec.material != 0)
		{
			if (auto sr = e.Get<SpriteRender>())
			{
				if (spec.layer != ParticleEmitter::kKeepLayer)
					sr->SetLayer(spec.layer);
				if (spec.material != 0)
					sr->SetMaterial(spec.material);
			}
		}
		return true;
	}

	void ParticleSystem::UpdateEmitters(float delta)
	{
		// Snapshot the list of emitters that finish this frame so we can destroy them
		// safely after iteration (registry mutation during view iteration is risky).
		std::vector<EntityId> toDestroy;

		// Convert the sampled rate (bursts/second) into a delay (seconds). A rate
		// of 0 or below maps to +infinity so the emitter idles until the user
		// raises the rate again, without flipping `playing`.
		auto sampleInterval = [](const ParticleEmitter& em) -> float
			{
				float rate = RandRange(em.burstRateMin, em.burstRateMax);
				if (rate <= 0.f) return std::numeric_limits<float>::infinity();
				return 1.f / rate;
			};

		auto view = Entity::View<ParticleEmitter, Transform>();
		view.each([&](Entity e, ParticleEmitter& em, Transform& tr)
			{
				if (!em.playing && !em.burstNow)
					return;

				Vec3f origin = tr.GetPosition();

				// Resolve this emitter's destroy-callback id. Register lazily and
				// re-register if the cached entry was already freed (its previous
				// generation of particles all died). Bounded: one entry per emitter.
				std::uint32_t cid = 0;
				if (em.onParticleDestroy)
				{
					if (em._callbackId == 0 ||
						m_destroyCallbacks.find(em._callbackId) == m_destroyCallbacks.end())
						em._callbackId = RegisterDestroyCallback(em.onParticleDestroy);
					cid = em._callbackId;
				}
				else
				{
					em._callbackId = 0;
				}

				// Manual one-off burst, regardless of timer / remainingBursts.
				if (em.burstNow)
				{
					em.burstNow = false;
					Burst(origin, em, cid);
				}

				if (!em.playing) return;
				if (em.remainingBursts == 0) return;

				// First Update after the emitter went live: fire one burst immediately
				// (no warm-up delay), then start the timer ticking. `_nextInterval` is
				// used purely as a sentinel: <0 = never run yet, >=0 = running.
				if (em._nextInterval < 0.f)
				{
					Burst(origin, em, cid);
					if (em.remainingBursts > 0)
						--em.remainingBursts;
					em._accum = 0.f;
					em._nextInterval = 0.f;
					if (em.remainingBursts == 0)
					{
						if (em.destroyOnFinish) toDestroy.push_back(e.GetId());
						return;
					}
				}

				em._accum += delta;

				// Sample the interval fresh each iteration so changes to burstRate
				// take effect immediately (including raising rate after a 0 pause).
				// Capped per frame to avoid runaway after long unpauses.
				int safety = 32;
				while (em.remainingBursts != 0 && safety-- > 0)
				{
					float interval = sampleInterval(em);
					if (!std::isfinite(interval))
					{
						// rate <= 0: drain the accumulator so we don't spam bursts
						// the moment the rate becomes positive again.
						em._accum = 0.f;
						break;
					}
					if (em._accum < interval) break;
					em._accum -= interval;
					Burst(origin, em, cid);
					if (em.remainingBursts > 0)
						--em.remainingBursts;
				}

				// Drop a freshly-registered entry that no particle ended up
				// referencing (e.g. burst hit the live-particle limit), so an idle
				// emitter doesn't keep a dead entry around.
				if (cid != 0)
				{
					auto it = m_destroyCallbacks.find(cid);
					if (it != m_destroyCallbacks.end() && it->second.refCount <= 0)
					{
						m_destroyCallbacks.erase(it);
						em._callbackId = 0;
					}
				}

				if (em.remainingBursts == 0 && em.destroyOnFinish)
					toDestroy.push_back(e.GetId());
			});

		for (EntityId id : toDestroy)
		{
			Entity(id).Destroy();
		}
	}
}
