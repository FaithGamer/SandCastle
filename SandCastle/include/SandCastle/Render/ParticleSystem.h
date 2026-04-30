#pragma once

#include "SandCastle/ECS/System.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Color.h"

namespace SandCastle
{
	class Sprite;

	/// @brief Built-in trajectory shapes Make() can build for you.
	enum class ParticleTraj
	{
		Straight,
		CubicIn,
		CubicOut,
		CubicInOut
	};

	/// @brief Engine system that spawns and updates Particle entities.
	/// Configure once with SetDefaultSprite()/SetLimit(), then call Make() per
	/// spawn. Particles are destroyed automatically when they reach t = 1.
	class ParticleSystem : public System
	{
	public:
		void Start() override;
		void Update() override;
		int GetUsedMethod() override;

		/// @brief Enable or disable the system. When disabled all particles are destroyed.
		void Activate(bool on);
		bool IsActive() const { return m_on; }

		/// @brief Maximum live particles. Any Make() call above the limit is dropped.
		void SetLimit(int limit) { m_limit = limit; }
		int GetLimit() const { return m_limit; }
		int GetCount() const { return m_count; }

		/// @brief Sprite used by Make() when no sprite is passed in explicitly.
		void SetDefaultSprite(Sprite* sprite) { m_defaultSprite = sprite; }
		Sprite* GetDefaultSprite() const { return m_defaultSprite; }

		/// @brief Spawn a particle moving from p1 to p2.
		/// Uses the system's default sprite. If none is set, returns an invalid Entity.
		Entity Make(Vec3f p1,
			Vec3f p2,
			float speed = 1.f,
			Color color = Color::White,
			ParticleTraj traj = ParticleTraj::Straight,
			float curveIntensity = 0.3f,
			float fade = 0.f,
			double(*easing)(double) = nullptr,
			float scale = 0.f);

		inline Entity Make(Vec2f p1,
			Vec2f p2,
			float speed = 1.f,
			Color color = Color::White,
			ParticleTraj traj = ParticleTraj::Straight,
			float curveIntensity = 0.3f,
			float fade = 0.f,
			double(*easing)(double) = nullptr,
			float scale = 0.f)
		{
			return Make(Vec3f(p1.x, p1.y, 0.f), Vec3f(p2.x, p2.y, 0.f),
				speed, color, traj, curveIntensity, fade, easing, scale);
		}

		/// @brief Spawn a particle using an explicit sprite (overrides the default).
		Entity MakeWithSprite(Sprite* sprite,
			Vec3f p1,
			Vec3f p2,
			float speed = 1.f,
			Color color = Color::White,
			ParticleTraj traj = ParticleTraj::Straight,
			float curveIntensity = 0.3f,
			float fade = 0.f,
			double(*easing)(double) = nullptr,
			float scale = 0.f);

	private:
		Sprite* m_defaultSprite = nullptr;
		bool m_on = true;
		int m_limit = 1000;
		int m_count = 0;
	};
}
