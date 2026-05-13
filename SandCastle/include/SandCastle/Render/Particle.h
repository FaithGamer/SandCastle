#pragma once

#include <vector>
#include <initializer_list>
#include "SandCastle/Render/Beziers.h"
#include "SandCastle/Render/Color.h"
#include "SandCastle/Render/Layer.h"
#include "SandCastle/Render/Material.h"

namespace SandCastle
{
	class Sprite;

	/// @brief Easing function pointer used to shape particle interpolation. See Easing namespace.
	using ParticleEasingFn = double(*)(double);

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

		inline ParticleEmitter& UseSprite(Sprite* s)              { sprite = s;                                 return *this; }
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
		inline ParticleEmitter& Fade(float f)                     { fade = f;                                   return *this; }
		inline ParticleEmitter& Tint(Color c)                     { colorA = c; colorB = c; colorChoices.clear(); return *this; }
		inline ParticleEmitter& Tint(Color a, Color b)            { colorA = a; colorB = b; colorChoices.clear(); return *this; }
		inline ParticleEmitter& Tint(std::initializer_list<WeightedColor> choices) { colorChoices.assign(choices);     return *this; }
		inline ParticleEmitter& Tint(std::vector<WeightedColor> choices)           { colorChoices = std::move(choices); return *this; }
		inline ParticleEmitter& EasingFn(ParticleEasingFn e)      { easing = e;                                 return *this; }
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
	};
}
