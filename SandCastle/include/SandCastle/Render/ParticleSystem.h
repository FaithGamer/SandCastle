#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include "SandCastle/ECS/System.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Color.h"
#include "SandCastle/Render/Particle.h"

namespace SandCastle
{
	class Sprite;

	/// @brief Lightweight wrapper returned by ParticleSystem::MakeEmitter().
	/// Stores only the Entity — the ParticleEmitter component pointer is fetched
	/// fresh on every setter call, so the handle stays valid across frames even
	/// if EnTT relocates components. Safe to store as a member.
	///
	/// Every chainable setter from ParticleEmitter is mirrored here and returns
	/// `EmitterHandle&`, so the entire chain uses `.` — no mix of `->` and `.`:
	/// @code
	/// sys(ParticleSystem)->MakeEmitter({x, y, 0.f})
	///     .UseSprite(s)
	///     .Tint(Palette::Green1)
	///     .Distance(40.f, 80.f)
	///     .CountPerBurst(8, 12)
	///     .Lifetime(0.6f, 1.0f)
	///     .BurstRate(10.f);
	/// @endcode
	/// `operator->` and `operator*` are still available for direct read access
	/// to the component (e.g. `h->countMin`). Use `entity.Destroy()` to remove
	/// the emitter.
	struct EmitterHandle
	{
		Entity entity{};

		inline ParticleEmitter* operator->() { return entity.Get<ParticleEmitter>(); }
		inline ParticleEmitter& operator*()  { return *entity.Get<ParticleEmitter>(); }
		inline bool Valid() { return entity.Valid() && entity.Get<ParticleEmitter>() != nullptr; }

		// Forwarders to ParticleEmitter's fluent setters. Each one re-resolves the
		// component pointer per call so no reference is held across statements.
#define SC_EMITTER_FWD0(NAME)         inline EmitterHandle& NAME()           { if (auto* em = entity.Get<ParticleEmitter>()) em->NAME();    return *this; }
#define SC_EMITTER_FWD1(NAME, T1)     inline EmitterHandle& NAME(T1 a)       { if (auto* em = entity.Get<ParticleEmitter>()) em->NAME(a);   return *this; }
#define SC_EMITTER_FWD2(NAME, T1, T2) inline EmitterHandle& NAME(T1 a, T2 b) { if (auto* em = entity.Get<ParticleEmitter>()) em->NAME(a, b); return *this; }

		SC_EMITTER_FWD1(UseSprite, Sprite*)
		inline EmitterHandle& UseSprite(std::initializer_list<Sprite*> ch) { if (auto* em = entity.Get<ParticleEmitter>()) em->UseSprite(ch);            return *this; }
		inline EmitterHandle& UseSprite(std::vector<Sprite*> ch)           { if (auto* em = entity.Get<ParticleEmitter>()) em->UseSprite(std::move(ch)); return *this; }
		SC_EMITTER_FWD1(OnLayer, LayerID)
		SC_EMITTER_FWD1(WithMaterial, MaterialID)
		SC_EMITTER_FWD1(Trajectory, ParticleTraj)
		SC_EMITTER_FWD2(Curve, float, float)
		SC_EMITTER_FWD1(Curve, float)
		inline EmitterHandle& Curve(std::initializer_list<float> ch) { if (auto* em = entity.Get<ParticleEmitter>()) em->Curve(ch);              return *this; }
		inline EmitterHandle& Curve(std::vector<float> ch)           { if (auto* em = entity.Get<ParticleEmitter>()) em->Curve(std::move(ch)); return *this; }
		SC_EMITTER_FWD1(CurveBothSides, bool)
		SC_EMITTER_FWD2(Distance, float, float)
		SC_EMITTER_FWD1(Distance, float)
		SC_EMITTER_FWD2(Angle, float, float)
		SC_EMITTER_FWD1(Angle, float)
		SC_EMITTER_FWD2(Lifetime, float, float)
		SC_EMITTER_FWD1(Lifetime, float)
		SC_EMITTER_FWD2(Scale, float, float)
		SC_EMITTER_FWD1(Scale, float)
		SC_EMITTER_FWD2(Spin, float, float)
		SC_EMITTER_FWD1(Spin, float)
		SC_EMITTER_FWD1(SpinBothSides, bool)
		SC_EMITTER_FWD1(Fade, float)
		SC_EMITTER_FWD1(Tint, Color)
		SC_EMITTER_FWD2(Tint, Color, Color)
		inline EmitterHandle& Tint(std::initializer_list<WeightedColor> ch) { if (auto* em = entity.Get<ParticleEmitter>()) em->Tint(ch);              return *this; }
		inline EmitterHandle& Tint(std::vector<WeightedColor> ch)           { if (auto* em = entity.Get<ParticleEmitter>()) em->Tint(std::move(ch)); return *this; }
		SC_EMITTER_FWD1(EasingFn, ParticleEasingFn)
		SC_EMITTER_FWD2(SpawnArea, float, float)
		inline EmitterHandle& SpawnArea(const Rect& area) { if (auto* em = entity.Get<ParticleEmitter>()) em->SpawnArea(area); return *this; }
		SC_EMITTER_FWD0(NoSpawnArea)
		inline EmitterHandle& OnParticleDestroy(std::function<void(const ParticleSignal&)> cb) { if (auto* em = entity.Get<ParticleEmitter>()) em->OnParticleDestroy(std::move(cb)); return *this; }
		SC_EMITTER_FWD2(BurstRate, float, float)
		SC_EMITTER_FWD1(BurstRate, float)
		SC_EMITTER_FWD2(CountPerBurst, int, int)
		SC_EMITTER_FWD1(CountPerBurst, int)
		SC_EMITTER_FWD1(Bursts, int)
		inline EmitterHandle& DestroyWhenDone(bool b = true) { if (auto* em = entity.Get<ParticleEmitter>()) em->DestroyWhenDone(b); return *this; }
		SC_EMITTER_FWD0(Play)
		SC_EMITTER_FWD0(Pause)
		SC_EMITTER_FWD0(BurstOnce)

#undef SC_EMITTER_FWD0
#undef SC_EMITTER_FWD1
#undef SC_EMITTER_FWD2
	};

	/// @brief Engine system that spawns and updates particles, and now drives
	/// ParticleEmitter components.
	///
	/// Two usage styles, both available concurrently:
	///   1. **Imperative** — call Make(p1, p2, ...) per particle to fire one-off shots.
	///   2. **Component-driven** — call MakeEmitter() to attach a configurable
	///      ParticleEmitter to an entity; the system bursts particles for you
	///      every frame based on the emitter's rules.
	///
	/// Configure once with SetDefaultSprite()/SetLimit(). Particles are destroyed
	/// automatically when they reach t = 1. Emitters honor the same global limit.
	class ParticleSystem : public System
	{
	public:
		void Start() override;
		void Update() override;
		int GetUsedMethod() override;

		/// @brief Enable or disable the system. When disabled all particles and emitter scheduling are cleared.
		void Activate(bool on);
		bool IsActive() const { return m_on; }

		/// @brief Maximum live particles. Any Make()/emitter burst above the limit is dropped.
		void SetLimit(int limit) { m_limit = limit; }
		int GetLimit() const { return m_limit; }
		int GetCount() const { return m_count; }

		/// @brief Sprite used by Make() and ParticleEmitter when no sprite is set explicitly.
		void SetDefaultSprite(Sprite* sprite) { m_defaultSprite = sprite; }
		Sprite* GetDefaultSprite() const { return m_defaultSprite; }

		// ============================================================
		//   Imperative API (kept stable for older projects)
		// ============================================================

		/// @brief Spawn a particle moving from p1 to p2. Uses the system's default sprite;
		/// returns an invalid Entity if no default sprite is set or if the live-particle
		/// count is already at the limit.
		/// @param speed t-rate multiplier — 1.0 ≈ 1 second to traverse p1→p2.
		/// Higher = faster = shorter lifetime. Particle dies when t reaches 1.
		/// @param fade signed alpha ramp as a fraction of lifetime: positive fades out
		/// over the final portion, negative fades in over the initial portion, 0 = no fade.
		/// @param easing optional curve applied to t before sampling the trajectory.
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
		/// Same parameter semantics as Make(); returns an invalid Entity if the
		/// live-particle count is already at the limit.
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

		// ============================================================
		//   Emitter API
		// ============================================================

		/// @brief Create a new entity at `position` with Transform + ParticleEmitter,
		/// then return an EmitterHandle for fluent setup. The handle stores only
		/// the entity; every chained setter re-resolves the component, so it's
		/// safe to keep around across frames.
		///
		/// Example: spawn green confetti every 100ms forever from (x, y):
		/// @code
		/// sys(ParticleSystem)->MakeEmitter({x, y, 0.f})
		///     .UseSprite(Assets::Get<Sprite>("square.png_0_0"))
		///     .Tint(Palette::Green1)
		///     .Distance(40.f, 80.f)
		///     .Angle(0.f, 360.f)
		///     .CountPerBurst(8, 12)
		///     .Lifetime(0.6f, 1.0f)
		///     .BurstRate(10.f)
		///     .Fade(0.4f)
		///     .Scale(1.5f, 2.5f);
		/// @endcode
		EmitterHandle MakeEmitter(Vec3f position = Vec3f(0.f, 0.f, 0.f));
		inline EmitterHandle MakeEmitter(Vec2f position)
		{
			return MakeEmitter(Vec3f(position.x, position.y, 0.f));
		}

		/// @brief Attach a ParticleEmitter to an existing entity that already has a Transform.
		/// Returns a handle whose `entity` is the passed-in one. Configure via the fluent setters.
		EmitterHandle MakeEmitter(Entity entity);

		/// @brief Spawn one burst at `position` using the supplied (temporary) emitter spec.
		/// Useful for one-shot effects when you don't need a persistent emitter entity.
		/// Returns the number of particles actually spawned (may be less than requested
		/// when the live-particle limit is reached).
		/// If `spec.onParticleDestroy` is set, it is registered in the runtime
		/// callback table for the lifetime of this burst's particles.
		int Burst(Vec3f position, const ParticleEmitter& spec);
		inline int Burst(Vec2f position, const ParticleEmitter& spec)
		{
			return Burst(Vec3f(position.x, position.y, 0.f), spec);
		}

	private:
		/// @brief Spawns a single particle around `origin` according to `spec`'s rules. Returns true on spawn.
		/// `cid` (0 = none) is stamped onto the particle and ref-counts the destroy-callback entry.
		bool SpawnFromEmitter(Vec3f origin, const ParticleEmitter& spec, std::uint32_t cid);
		/// @brief Shared burst path: spawns `count` particles, each ref-counting `cid`.
		int Burst(Vec3f position, const ParticleEmitter& spec, std::uint32_t cid);
		/// @brief Drives every ParticleEmitter component: accumulates time, fires bursts.
		void UpdateEmitters(float delta);

		/// @brief Copy `fn` into the table, return its new id (0 if `fn` is empty).
		/// The entry starts at refCount 0 — callers must ref it via spawned particles
		/// or release it explicitly if nothing ended up referencing it.
		std::uint32_t RegisterDestroyCallback(const std::function<void(const ParticleSignal&)>& fn);
		/// @brief Decrement an entry's refCount; erase it when it reaches 0. No-op for id 0.
		void ReleaseDestroyCallback(std::uint32_t id);

		/// @brief Runtime destroy-callback table. Keyed by a small id stored on
		/// emitters (cached) and particles (per spawn). Ref-counted by the number
		/// of live particles pointing at it, so a callback outlives its emitter and
		/// is freed once its last particle dies.
		struct DestroyCallback
		{
			std::function<void(const ParticleSignal&)> fn;
			int refCount = 0;
		};
		std::unordered_map<std::uint32_t, DestroyCallback> m_destroyCallbacks;
		std::uint32_t m_nextCallbackId = 1;

		Sprite* m_defaultSprite = nullptr;
		bool m_on = true;
		int m_limit = 1000;
		int m_count = 0;
	};
}
