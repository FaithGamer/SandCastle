#pragma once

#include <glad/glad.h>
#include <vector>
#include <atomic>
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Render/Color.h"
#include "SandCastle/Render/Layer.h"
#include "SandCastle/Render/RenderTarget.h"

namespace SandCastle
{
	class Shader;
	class Sprite;

	/// @brief Opaque handle to a GpuParticleSystem owned by Renderer2D.
	using GpuParticleSystemId = uint32_t;
	/// @brief Returned by CreateGpuParticleSystem on failure / used as a null handle.
	constexpr GpuParticleSystemId GpuParticleSystemInvalid = (GpuParticleSystemId)-1;

	/// @brief One particle to inject into a GpuParticleSystem (main-thread side).
	/// After spawn the CPU never touches it again: the GPU advances `pos` by
	/// `velocity`, and `velocity` by the system's gravity/drag, every frame. Color
	/// lerps colorStart -> colorEnd across [0, life]. `seed` is a free per-particle
	/// random the shaders may use. Positions/sizes are world units; angles degrees.
	struct GpuParticleSpawn
	{
		Vec3f pos = { 0.f, 0.f, 0.f };
		Vec2f velocity = { 0.f, 0.f };
		float life = 1.f;             // seconds; <= 0 spawns nothing
		float sizeStart = 1.f;        // world-space quad size at spawn
		float sizeEnd = 1.f;          // world-space quad size at death (lerped via the system's sizeEase)
		float rotation = 0.f;         // degrees
		float rotationSpeed = 0.f;    // degrees / second
		float seed = 0.f;             // free per-particle random (jitter phase, etc.)
		Color colorStart = Color(255, 255, 255, 255);
		Color colorEnd = Color(255, 255, 255, 0);
	};

	/// @brief How a GpuParticleSystem's particles blend into their layer.
	enum class GpuParticleBlend
	{
		Alpha,        // straight alpha (over): particles occlude, no glow, no white-out
		Additive,     // dst += srcAlpha*src: light SUMS; dense overlap clamps hard to white (harsh glow)
		SoftAdditive, // additive that rolls off toward white (dst *= 1-src as it brightens) instead of
		              // clamping — keeps the fiery colour in dense regions. Best for fire / embers / glow.
	};

	/// @brief Easing applied to a particle's size as it lerps sizeStart -> sizeEnd over its life.
	enum class GpuParticleSizeEase
	{
		Linear,   // constant rate
		QuadIn,   // slow then fast (t*t)
		QuadOut,  // fast then slow (1-(1-t)^2)
	};

	/// @brief Creation parameters for a GpuParticleSystem.
	struct GpuParticleSystemDesc
	{
		uint32_t capacity = 8192;       // max concurrent particles (ring-allocated)
		LayerID layer = 0;              // render layer the particles composite onto (use a real layer >= 1)
		const Sprite* sprite = nullptr; // atlas region + texture shared by every particle
		Vec2f gravity = { 0.f, 0.f };   // world units / s^2 added to velocity each second
		float drag = 0.f;               // per-second velocity damping (0 = none)
		GpuParticleBlend blend = GpuParticleBlend::Alpha;        // how particles composite (see GpuParticleBlend)
		GpuParticleSizeEase sizeEase = GpuParticleSizeEase::Linear; // easing for the sizeStart->sizeEnd lerp
		float jitterAmplitude = 0.f;    // world units of perpendicular wobble (0 = none)
		float jitterFrequency = 0.f;    // wobble speed in radians/second
	};

	/// @brief The two GLSL programs shared by every GpuParticleSystem. Compiled once,
	/// lazily, on the render thread the first time a system is created (so games that
	/// never use GPU particles pay nothing). Owned by Renderer2D.
	struct GpuParticleShaders
	{
		Shader* update = nullptr;   // transform-feedback simulation program
		Shader* render = nullptr;   // point -> camera-facing quad (geometry shader)
		GLint uDt = -1, uGravity = -1, uDrag = -1;            // update-program uniforms
		GLint uAtlas = -1, uTexture = -1;                     // render-program uniforms
		GLint uSizeEase = -1, uJitterAmp = -1, uJitterFreq = -1; // render-program uniforms
		bool ready = false;
	};

	/// @brief A GPU-resident particle pool simulated with transform feedback.
	///
	/// Particle state (position, velocity, age, life, colors) lives entirely in two
	/// ping-pong VBOs. Each frame the GPU advances every particle in an update pass
	/// (vertex shader + transform feedback, rasterizer discarded) and draws the
	/// survivors as camera-facing quads expanded by a geometry shader in a render
	/// pass. The CPU only injects spawns (ring-allocated into the buffer) and never
	/// reads or writes per-particle state afterwards -- so there are no ECS entities
	/// and no global particle cap. Purely cosmetic: no CPU readback, no per-particle
	/// destroy callbacks.
	///
	/// Created and owned by Renderer2D. Every GL method runs on the render thread;
	/// Emit()/SwapSpawns() are the only main-thread entry points and use a
	/// double-buffered spawn queue drained on the render thread (same pattern as the
	/// quad RenderQueue), so no locking is needed.
	class GpuParticleSystem
	{
	public:
		GpuParticleSystem(const GpuParticleSystemDesc& desc);

		/// @brief Compile the shared update/render programs (render thread). Returns a
		/// fully-populated GpuParticleShaders (uniform locations cached, scene UBO bound).
		static GpuParticleShaders CompileShaders(GLuint sceneUniformBinding);

		// ---- main thread ----
		/// @brief Queue a particle for the next frame. Cheap; thread-safe vs the render thread.
		void Emit(const GpuParticleSpawn& spawn);
		/// @brief Flip the double-buffered spawn queue. Called by Renderer2D::Process while the render thread is idle.
		void SwapSpawns();
		LayerID GetLayer() const { return m_layer; }
		uint32_t GetCapacity() const { return m_capacity; }

		// Live tweaks (main thread). These members are read by the render thread each
		// frame; the writes are unsynchronised on purpose — fine for debug/tuning (a
		// torn vec2 gravity is at worst one cosmetically-off frame).
		void SetGravity(const Vec2f& gravity) { m_gravity = gravity; }
		void SetDrag(float drag) { m_drag = drag; }
		void SetBlend(GpuParticleBlend blend) { m_blend = blend; }
		void SetSizeEase(GpuParticleSizeEase ease) { m_sizeEase = ease; }
		void SetJitter(float amplitude, float frequency) { m_jitterAmp = amplitude; m_jitterFreq = frequency; }

		// ---- render thread ----
		void InitGL();                                                 // allocate buffers/VAOs (zeroed => all slots dead)
		void ApplySpawns();                                            // upload drained spawns into the current state buffer
		void Update(const GpuParticleShaders& shaders, float dt);      // transform-feedback simulation step
		void Render(const GpuParticleShaders& shaders, sptr<RenderTarget> target); // draw into the layer's framebuffer
		void DestroyGL();

	private:
		// Matches the interleaved attribute layout consumed by both programs:
		// 5 * vec4, tightly packed (80 bytes). Order is load-bearing.
		struct GpuParticle
		{
			Vec4f posAge;       // pos.xyz, age
			Vec4f velLifeSize;  // vel.xy, life, size
			Vec4f colorStart;   // rgba 0..1
			Vec4f colorEnd;     // rgba 0..1
			Vec4f rot;          // rotation(deg), rotationSpeed(deg/s), seed, _
		};

		uint32_t m_capacity;
		LayerID m_layer;
		GLuint m_textureID = 0;
		Vec4f m_atlas = { 0.f, 0.f, 1.f, 1.f }; // (uMin, vMin, uMax, vMax)
		Vec2f m_gravity = { 0.f, 0.f };
		float m_drag = 0.f;
		GpuParticleBlend m_blend = GpuParticleBlend::Alpha;
		GpuParticleSizeEase m_sizeEase = GpuParticleSizeEase::Linear;
		float m_jitterAmp = 0.f;
		float m_jitterFreq = 0.f;

		GLuint m_vbo[2] = { 0, 0 };
		GLuint m_vao[2] = { 0, 0 };
		int m_front = 0;        // index of the buffer holding the current state
		uint32_t m_cursor = 0;  // ring write head for spawns
		bool m_glReady = false;

		// Double-buffered spawn queue: Emit() writes the back buffer, the render
		// thread drains the front in ApplySpawns(), SwapSpawns() flips them.
		std::vector<GpuParticle> m_spawns[2];
		std::atomic<int> m_spawnCurrent{ 0 };
	};
}
