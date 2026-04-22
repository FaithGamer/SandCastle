#include "pch.h"
#include "SandCastle/Render/ParticleSystem.h"
#include "SandCastle/Render/Particle.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Transform.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Core/Easing.h"
#include "SandCastle/Core/Time.h"

namespace SandCastle
{
	void ParticleSystem::Start()
	{
	}

	int ParticleSystem::GetUsedMethod()
	{
		return System::Method::Updt;
	}

	void ParticleSystem::Update()
	{
		auto view = Entity::View<Particle, Transform, SpriteRender>();
		float delta = (float)Time::Delta();
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

				Vec2f pos = p.trajectory.Step(t);
				float z = tr.GetPosition().z;
				tr.SetPosition(pos.x, pos.y, z);

				if (p.t >= 1.f)
				{
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
		return entt;
	}
}
