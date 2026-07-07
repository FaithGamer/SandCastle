#pragma once

#include <vector>
#include <initializer_list>
#include <functional>
#include <cstdint>
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Beziers.h"
#include "SandCastle/Render/Color.h"
#include "SandCastle/Render/Layer.h"
#include "SandCastle/Render/Material.h"
#include "SandCastle/Render/Rect.h"

namespace SandCastle
{
	class Sprite;

	/// @brief Easing function pointer used to shape particle interpolation. See Easing namespace.
	using ParticleEasingFn = double(*)(double);

	/// @brief Payload passed to a ParticleEmitter::onParticleDestroy callback.
	/// Currently carries the particle's final world position; wrapped in a struct
	/// so new fields can be added later without changing the callback signature.
	struct ParticleSignal
	{
		Vec3f position;
	};

	/// @brief Color + relative weight used by ParticleEmitter::Tint() for weighted random pick.
	/// Implicitly constructible from a plain Color (weight defaults to 1) so a weighted list
	/// can mix raw colors with `{color, weight}` pairs: `{ Palette::Red, { Palette::Blue, 3.f } }`.
	struct WeightedColor
	{
		Color color = Color::White;
		float weight = 1.f;

		WeightedColor() = default;
		WeightedColor(Color c) : color(c), weight(1.f) {}
		WeightedColor(Color c, float w) : color(c), weight(w) {}
	};

	/// @brief Built-in trajectory shapes ParticleSystem::Make / ParticleEmitter can build for you.
	enum class ParticleTraj
	{
		Straight,
		CubicIn,
		CubicOut,
		CubicInOut
	};

	/// @brief ECS component holding per-particle state used by ParticleSystem.
	/// `t` walks from 0 to 1 along `trajectory`, modulated by `easing`. `fade`
	/// fades out alpha over the lifetime, `scale` linearly grows/shrinks size,
	/// `rotSpeed` spins the sprite (degrees/second, sign = direction).
	struct Particle
	{
		float t = 0.f;
		float speed = 1.f;
		float fade = 0.f;
		float scale = 0.f;
		float rotSpeed = 0.f;
		float rot = 0.f; // current angle in degrees, advanced by rotSpeed
		Beziers trajectory;
		ParticleEasingFn easing = nullptr;
		/// @brief Handle into ParticleSystem's destroy-callback table (0 = none). The
		/// callback lives in that table, ref-counted by live particles, so it fires
		/// independently of the emitter's lifetime. Managed by ParticleSystem.
		std::uint32_t destroyCb = 0;
	};

	/// @brief ECS component that turns its entity into a particle emitter.
	/// ParticleSystem reads the emitter every frame, accumulates time and spawns
	/// bursts according to the rules below. Every numeric rule has a min/max
	/// pair (set both to the same value for a fixed result). Spawned particles
	/// are independent entities, so the emitter can move, rotate or be destroyed
	/// without affecting particles already in flight.
	///
	/// Pair with a Transform on the same entity — the transform's world position
	/// is sampled as the burst origin at spawn time. Build it through
	/// ParticleSystem::MakeEmitter() for a one-liner setup, or add the component
	/// manually with Entity::Add<ParticleEmitter>() and configure the fields.
	/// All setter methods return `*this` so they can be chained.
	struct ParticleEmitter
	{
		// === Visual ===

		/// @brief Sprite each particle uses. nullptr falls back to ParticleSystem::GetDefaultSprite().
		Sprite* sprite = nullptr;
		/// @brief Discrete sprite options. When non-empty, supersedes `sprite` — the
		/// system picks one uniformly at random per particle. Entries must be
		/// non-null (filter missing frames before assigning).
		std::vector<Sprite*> spriteChoices;
		/// @brief Render layer override applied to spawned particles. Use kKeepLayer to keep SpriteRender::defaultLayer.
		LayerID layer = kKeepLayer;
		/// @brief Material override applied to spawned particles. 0 = default material.
		MaterialID material = 0;

		/// @brief Trajectory style used by every spawned particle.
		ParticleTraj traj = ParticleTraj::Straight;
		/// @brief Bezier curve intensity, sampled per particle in [curveMin, curveMax].
		/// Ignored when `curveChoices` is non-empty.
		float curveMin = 0.f;
		float curveMax = 0.f;
		/// @brief When true, the sampled curve is randomly flipped left/right per particle.
		/// Ignored when `curveChoices` is non-empty (encode sides explicitly in the list).
		bool  curveBothSides = true;
		/// @brief Discrete curve options. When non-empty, supersedes curveMin/curveMax
		/// and curveBothSides — the system picks one value uniformly at random per
		/// particle. Example: `{ -0.5f, 0.5f }` to alternate sharp left/right curves.
		std::vector<float> curveChoices;

		// === Travel ===

		/// @brief Distance from emitter to particle end-point, sampled per particle in world units.
		float distanceMin = 50.f;
		float distanceMax = 100.f;
		/// @brief Emission cone in degrees. 0° points up; sweeps clockwise. Use 0..360 for a full circle.
		float angleMin = 0.f;
		float angleMax = 360.f;

		/// @brief Particle lifetime in seconds (controls Particle::speed = 1 / lifetime).
		float lifetimeMin = 1.f;
		float lifetimeMax = 1.f;

		/// @brief Scale modulation passed to Particle::scale (0 = no scaling, >1 = grow-and-shrink envelope).
		float scaleMin = 0.f;
		float scaleMax = 0.f;

		/// @brief Spin in degrees/second, sampled per particle in [spinMin, spinMax].
		/// With `spinBothSides` (default) the sampled value is randomly flipped per
		/// particle, so Spin(720, 1080) spins fast in BOTH directions with no slow
		/// spinners — mirrors curveBothSides. A non-zero spin also randomizes the
		/// particle's initial angle in [0, 360) so a burst doesn't start
		/// phase-locked. 0 (default) = no spin.
		float spinMin = 0.f;
		float spinMax = 0.f;
		bool  spinBothSides = true;

		/// @brief Alpha ramp applied to every particle. >0 fades out near end of life, <0 fades in at start.
		float fade = 0.f;

		/// @brief Tint range — each particle is colored Lerp(colorA, colorB, rand[0,1]) component-wise.
		/// Ignored when `colorChoices` is non-empty.
		Color colorA = Color::White;
		Color colorB = Color::White;
		/// @brief Discrete weighted color options. When non-empty, supersedes the
		/// colorA/colorB gradient — the system picks one entry per particle with
		/// probability proportional to its weight. Example: 50% red, 30% blue, 20% green:
		/// `{ { Palette::Red, 5.f }, { Palette::Blue, 3.f }, { Palette::Green, 2.f } }`.
		/// Weights are relative — `{ Red, Blue }` (defaults to weight 1 each) gives a 50/50.
		std::vector<WeightedColor> colorChoices;

		/// @brief Easing curve applied to t before sampling the trajectory (shared across particles).
		ParticleEasingFn easing = nullptr;

		/// @brief Optional callback fired once per particle, with that particle's final
		/// world position, just before it is destroyed at end of life. ParticleSystem
		/// copies it once into a ref-counted runtime table (not per particle); particles
		/// hold only a small id into that table. The callback therefore keeps firing for
		/// in-flight particles even after the emitter is destroyed — it lives until the
		/// last referencing particle dies. Also works with the temporary-spec Burst()
		/// overload. Leave empty (default) for no callback. Set via OnParticleDestroy();
		/// a handy "do X where the particle landed" trigger.
		std::function<void(const ParticleSignal&)> onParticleDestroy;

		// === Spawn area ===

		/// @brief Spawn-area box, expressed as an offset rect relative to the emitter's
		/// Transform position. Each particle samples its own uniform random point inside
		/// this box at spawn time, so a single burst scatters across the whole area
		/// instead of all originating from one point. A zero-size rect (the default)
		/// means point emission at the emitter origin. Configure via SpawnArea() — pass
		/// a (width, height) for a box centered on the emitter, or a Rect for an
		/// arbitrary (possibly off-center) offset box.
		Rect spawnArea = Rect(0.f, 0.f, 0.f, 0.f);

		// === Burst scheduling ===

		/// @brief Bursts per second, sampled per burst in [burstRateMin, burstRateMax].
		/// Internally converted to a delay (1 / rate). A rate of 0 (or negative) pauses
		/// emission without changing `playing` — set the rate back to >0 to resume.
		/// The very first burst after the emitter becomes live fires with no delay.
		float burstRateMin = 5.f;
		float burstRateMax = 5.f;
		/// @brief Particles spawned per burst, sampled per burst in [countMin, countMax].
		int countMin = 1;
		int countMax = 1;

		/// @brief When false, the emitter is paused (no time accumulates, no bursts fire).
		bool playing = true;
		/// @brief -1 = infinite (default). Otherwise the emitter stops after this many bursts.
		int  remainingBursts = -1;
		/// @brief Destroy the emitter's entity when remainingBursts reaches 0. No-op for infinite emitters.
		bool destroyOnFinish = false;
		/// @brief Set to true to force a burst on the next ParticleSystem::Update, regardless of timer. Auto-resets.
		bool burstNow = false;

		/// @brief Sentinel for `layer` meaning "do not override the default render layer".
		static constexpr LayerID kKeepLayer = (LayerID)-1;

		// === Fluent setters — all return *this for chaining ===

		inline ParticleEmitter& UseSprite(Sprite* s)              { sprite = s;                   spriteChoices.clear(); return *this; }
		inline ParticleEmitter& UseSprite(std::initializer_list<Sprite*> choices) { spriteChoices.assign(choices);     return *this; }
		inline ParticleEmitter& UseSprite(std::vector<Sprite*> choices)           { spriteChoices = std::move(choices); return *this; }
		inline ParticleEmitter& OnLayer(LayerID l)                { layer = l;                                  return *this; }
		inline ParticleEmitter& WithMaterial(MaterialID m)        { material = m;                               return *this; }
		inline ParticleEmitter& Trajectory(ParticleTraj t)        { traj = t;                                   return *this; }
		inline ParticleEmitter& Curve(float mn, float mx)         { curveMin = mn; curveMax = mx; curveChoices.clear(); return *this; }
		inline ParticleEmitter& Curve(float v)                    { curveMin = v;  curveMax = v;  curveChoices.clear(); return *this; }
		inline ParticleEmitter& CurveBothSides(bool b)            { curveBothSides = b;                         return *this; }
		inline ParticleEmitter& Curve(std::initializer_list<float> choices) { curveChoices.assign(choices);     return *this; }
		inline ParticleEmitter& Curve(std::vector<float> choices)           { curveChoices = std::move(choices); return *this; }
		inline ParticleEmitter& Distance(float mn, float mx)      { distanceMin = mn; distanceMax = mx;         return *this; }
		inline ParticleEmitter& Distance(float v)                 { distanceMin = v;  distanceMax = v;          return *this; }
		inline ParticleEmitter& Angle(float mn, float mx)         { angleMin = mn; angleMax = mx;               return *this; }
		inline ParticleEmitter& Angle(float v)                    { angleMin = v;  angleMax = v;                return *this; }
		inline ParticleEmitter& Lifetime(float mn, float mx)      { lifetimeMin = mn; lifetimeMax = mx;         return *this; }
		inline ParticleEmitter& Lifetime(float v)                 { lifetimeMin = v;  lifetimeMax = v;          return *this; }
		inline ParticleEmitter& Scale(float mn, float mx)         { scaleMin = mn; scaleMax = mx;               return *this; }
		inline ParticleEmitter& Scale(float v)                    { scaleMin = v;  scaleMax = v;                return *this; }
		inline ParticleEmitter& Spin(float mn, float mx)          { spinMin = mn; spinMax = mx;                 return *this; }
		inline ParticleEmitter& Spin(float v)                     { spinMin = v;  spinMax = v;                  return *this; }
		inline ParticleEmitter& SpinBothSides(bool b)             { spinBothSides = b;                          return *this; }
		inline ParticleEmitter& Fade(float f)                     { fade = f;                                   return *this; }
		inline ParticleEmitter& Tint(Color c)                     { colorA = c; colorB = c; colorChoices.clear(); return *this; }
		inline ParticleEmitter& Tint(Color a, Color b)            { colorA = a; colorB = b; colorChoices.clear(); return *this; }
		inline ParticleEmitter& Tint(std::initializer_list<WeightedColor> choices) { colorChoices.assign(choices);     return *this; }
		inline ParticleEmitter& Tint(std::vector<WeightedColor> choices)           { colorChoices = std::move(choices); return *this; }
		inline ParticleEmitter& EasingFn(ParticleEasingFn e)      { easing = e;                                 return *this; }
		inline ParticleEmitter& SpawnArea(float w, float h)       { spawnArea = Rect(-w * 0.5f, h * 0.5f, w, h); return *this; }
		inline ParticleEmitter& SpawnArea(const Rect& area)       { spawnArea = area;                           return *this; }
		inline ParticleEmitter& NoSpawnArea()                     { spawnArea = Rect(0.f, 0.f, 0.f, 0.f);       return *this; }
		inline ParticleEmitter& OnParticleDestroy(std::function<void(const ParticleSignal&)> cb) { onParticleDestroy = std::move(cb); return *this; }
		inline ParticleEmitter& BurstRate(float mn, float mx)     { burstRateMin = mn; burstRateMax = mx;       return *this; }
		inline ParticleEmitter& BurstRate(float v)                { burstRateMin = v;  burstRateMax = v;        return *this; }
		inline ParticleEmitter& CountPerBurst(int mn, int mx)     { countMin = mn; countMax = mx;               return *this; }
		inline ParticleEmitter& CountPerBurst(int v)              { countMin = v;  countMax = v;                return *this; }
		inline ParticleEmitter& Bursts(int n)                     { remainingBursts = n;                        return *this; }
		inline ParticleEmitter& DestroyWhenDone(bool b = true)    { destroyOnFinish = b;                        return *this; }
		inline ParticleEmitter& Play()                            { playing = true;                             return *this; }
		inline ParticleEmitter& Pause()                           { playing = false;                            return *this; }
		inline ParticleEmitter& BurstOnce()                       { burstNow = true;                            return *this; }

		// Internal scheduling state — managed by ParticleSystem.
		float _accum = 0.f;
		float _nextInterval = -1.f;
		/// @brief Cached id of this emitter's onParticleDestroy entry in
		/// ParticleSystem's table (0 = not registered). Managed by ParticleSystem.
		std::uint32_t _callbackId = 0;
	};
}
