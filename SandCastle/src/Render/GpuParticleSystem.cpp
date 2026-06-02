#include "pch.h"

#include <algorithm>
#include <cstring>
#include "SandCastle/Render/GpuParticleSystem.h"
#include "SandCastle/Render/Shader.h"
#include "SandCastle/Render/Sprite.h"
#include "SandCastle/Render/Texture.h"
#include "SandCastle/Core/Log.h"

namespace SandCastle
{
	//------------------------------------------------------------------------------
	// GLSL (compiled from source strings, like Window's cursor shader — these are
	// engine-internal and never live in the asset folder).
	//------------------------------------------------------------------------------

	// Update pass: pure simulation. Vertex shader integrates one particle and writes
	// the next state through transform feedback; rasterizer is discarded so the
	// fragment stage is a stub.
	static const char* kUpdateVS = R"(#version 330 core
layout(location = 0) in vec4 iPosAge;
layout(location = 1) in vec4 iVelLifeSize;
layout(location = 2) in vec4 iColorStart;
layout(location = 3) in vec4 iColorEnd;
layout(location = 4) in vec4 iRot;

uniform float uDt;
uniform vec2  uGravity;
uniform float uDrag;

out vec4 oPosAge;
out vec4 oVelLifeSize;
out vec4 oColorStart;
out vec4 oColorEnd;
out vec4 oRot;

void main()
{
    float life = iVelLifeSize.z;
    float age  = iPosAge.w;
    vec2  vel  = iVelLifeSize.xy;
    vec3  pos  = iPosAge.xyz;
    float rotation = iRot.x;

    // Dead / never-spawned slots (life <= 0) are frozen so they never drift; the
    // CPU ring overwrites them in place on reuse.
    if (life > 0.0)
    {
        age += uDt;
        vel += uGravity * uDt;
        vel *= max(0.0, 1.0 - uDrag * uDt);
        pos.xy += vel * uDt;
        rotation += iRot.y * uDt;
    }

    oPosAge      = vec4(pos, age);
    oVelLifeSize = vec4(vel, life, iVelLifeSize.w);
    oColorStart  = iColorStart;
    oColorEnd    = iColorEnd;
    oRot         = vec4(rotation, iRot.y, iRot.z, iRot.w);
}
)";

	// Stub fragment for the update program (rasterizer discard keeps it from running,
	// but the program must still link).
	static const char* kUpdateFS = R"(#version 330 core
void main() {}
)";

	// Render pass: pass particle state through to the geometry shader.
	static const char* kRenderVS = R"(#version 330 core
layout(location = 0) in vec4 iPosAge;
layout(location = 1) in vec4 iVelLifeSize;
layout(location = 2) in vec4 iColorStart;
layout(location = 3) in vec4 iColorEnd;
layout(location = 4) in vec4 iRot;

out vec3  vPos;
out float vAge;
out float vLife;
out float vSizeStart;
out float vSizeEnd;
out float vRot;
out float vSeed;
out vec2  vVel;
out vec4  vColorStart;
out vec4  vColorEnd;

void main()
{
    vPos        = iPosAge.xyz;
    vAge        = iPosAge.w;
    vLife       = iVelLifeSize.z;
    vSizeStart  = iVelLifeSize.w;
    vSizeEnd    = iRot.w;
    vRot        = iRot.x;
    vSeed       = iRot.z;
    vVel        = iVelLifeSize.xy;
    vColorStart = iColorStart;
    vColorEnd   = iColorEnd;
}
)";

	// Expand each live point into a camera-facing quad. Dead/empty particles emit no
	// vertices (culled here). camProjView comes from the shared scene UBO, exactly
	// like the quad shaders.
	static const char* kRenderGS = R"(#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3  vPos[];
in float vAge[];
in float vLife[];
in float vSizeStart[];
in float vSizeEnd[];
in float vRot[];
in float vSeed[];
in vec2  vVel[];
in vec4  vColorStart[];
in vec4  vColorEnd[];

layout(std140) uniform scene
{
    mat4  camProjView;
    float camZoom;
    float camAspectRatio;
    vec2  winSize;
    vec2  targetSize;
    int   cropMask;
    float reduction;
};

uniform vec4  uAtlas;       // (uMin, vMin, uMax, vMax)
uniform int   uSizeEase;    // 0 linear, 1 quadIn, 2 quadOut
uniform float uJitterAmp;   // world units of perpendicular wobble
uniform float uJitterFreq;  // wobble speed (radians/sec)

out vec2 gUv;
out vec4 gColor;

void main()
{
    float life = vLife[0];
    float age  = vAge[0];
    if (life <= 0.0 || age < 0.0 || age >= life)
        return; // dead / empty -> cull

    float t = age / life;
    gColor = mix(vColorStart[0], vColorEnd[0], t);

    // Size lerp sizeStart -> sizeEnd with selectable easing.
    float te = t;
    if (uSizeEase == 1)      te = t * t;          // QuadIn  (slow then fast)
    else if (uSizeEase == 2) te = t * (2.0 - t);  // QuadOut (fast then slow)
    float size = mix(vSizeStart[0], vSizeEnd[0], te);

    float h   = 0.5 * size;
    float rad = radians(vRot[0]);
    float c   = cos(rad);
    float s   = sin(rad);
    vec2 right = vec2(c, s) * h;
    vec2 up    = vec2(-s, c) * h;
    vec3 p     = vPos[0];

    // Perpendicular jitter: oscillate across the direction of travel. A two-sine
    // sum + per-particle seed phase reads as organic noise across the swarm,
    // while each particle wobbles smoothly. Render-only (doesn't accumulate in sim).
    if (uJitterAmp > 0.0)
    {
        vec2 vel = vVel[0];
        vec2 dir = (dot(vel, vel) > 1e-6) ? normalize(vel) : vec2(0.0, 1.0);
        vec2 perp = vec2(-dir.y, dir.x);
        float ph = vSeed[0] * 6.2831853;
        float j = sin(uJitterFreq * age + ph) + 0.5 * sin(uJitterFreq * 2.17 * age + ph * 1.7);
        p.xy += perp * (uJitterAmp * j);
    }

    // Match the quad shader's world->clip transform EXACTLY: positions are scaled
    // by reduction*camZoom before camProjView (which is built for that scaled
    // space). Skipping this magnifies everything by 1/(reduction*camZoom).
    float worldScale = reduction * camZoom;

    gl_Position = camProjView * vec4(vec3(p.xy - right - up, p.z) * worldScale, 1.0);
    gUv = vec2(uAtlas.x, uAtlas.y); EmitVertex();

    gl_Position = camProjView * vec4(vec3(p.xy + right - up, p.z) * worldScale, 1.0);
    gUv = vec2(uAtlas.z, uAtlas.y); EmitVertex();

    gl_Position = camProjView * vec4(vec3(p.xy - right + up, p.z) * worldScale, 1.0);
    gUv = vec2(uAtlas.x, uAtlas.w); EmitVertex();

    gl_Position = camProjView * vec4(vec3(p.xy + right + up, p.z) * worldScale, 1.0);
    gUv = vec2(uAtlas.z, uAtlas.w); EmitVertex();

    EndPrimitive();
}
)";

	static const char* kRenderFS = R"(#version 330 core
in vec2 gUv;
in vec4 gColor;

uniform sampler2D uTexture;

out vec4 FragColor;

void main()
{
    vec4 col = texture(uTexture, gUv) * gColor;
    if (col.a < 0.003)
        discard;
    FragColor = col;
}
)";

	//------------------------------------------------------------------------------

	GpuParticleShaders GpuParticleSystem::CompileShaders(GLuint sceneUniformBinding)
	{
		GpuParticleShaders s;

		s.update = new Shader(kUpdateVS, kUpdateFS,
			{ "oPosAge", "oVelLifeSize", "oColorStart", "oColorEnd", "oRot" }, true, "gpu_particle_update");
		s.render = new Shader(kRenderVS, kRenderGS, kRenderFS, "gpu_particle_render");

		s.uDt = s.update->GetUniformLocation("uDt");
		s.uGravity = s.update->GetUniformLocation("uGravity");
		s.uDrag = s.update->GetUniformLocation("uDrag");

		s.uAtlas = s.render->GetUniformLocation("uAtlas");
		s.uTexture = s.render->GetUniformLocation("uTexture");
		s.uSizeEase = s.render->GetUniformLocation("uSizeEase");
		s.uJitterAmp = s.render->GetUniformLocation("uJitterAmp");
		s.uJitterFreq = s.render->GetUniformLocation("uJitterFreq");
		s.render->BindUniformBlock("scene", (GLint)sceneUniformBinding);

		s.ready = true;
		return s;
	}

	GpuParticleSystem::GpuParticleSystem(const GpuParticleSystemDesc& desc)
	{
		m_capacity = desc.capacity ? desc.capacity : 1;
		m_layer = desc.layer;
		m_gravity = desc.gravity;
		m_drag = desc.drag;
		m_blend = desc.blend;
		m_sizeEase = desc.sizeEase;
		m_jitterAmp = desc.jitterAmplitude;
		m_jitterFreq = desc.jitterFrequency;
		if (desc.sprite)
		{
			m_atlas = desc.sprite->GetUVs();
			if (desc.sprite->GetTexture())
				m_textureID = desc.sprite->GetTexture()->GetId();
		}
	}

	void GpuParticleSystem::Emit(const GpuParticleSpawn& spawn)
	{
		if (spawn.life <= 0.f)
			return;

		// Layout: velLifeSize.w = sizeStart, rot.w = sizeEnd (the GS lerps between them).
		GpuParticle p;
		p.posAge = Vec4f{ spawn.pos.x, spawn.pos.y, spawn.pos.z, 0.f };
		p.velLifeSize = Vec4f{ spawn.velocity.x, spawn.velocity.y, spawn.life, spawn.sizeStart };
		p.colorStart = spawn.colorStart; // implicit Color -> Vec4f (0..1)
		p.colorEnd = spawn.colorEnd;
		p.rot = Vec4f{ spawn.rotation, spawn.rotationSpeed, spawn.seed, spawn.sizeEnd };

		// Producer side of the double buffer: write the buffer the render thread is
		// NOT currently draining.
		m_spawns[m_spawnCurrent.load() ^ 1].push_back(p);
	}

	void GpuParticleSystem::SwapSpawns()
	{
		m_spawnCurrent.store(m_spawnCurrent.load() ^ 1);
	}

	void GpuParticleSystem::InitGL()
	{
		// The interleaved attribute layout is load-bearing: 5 tightly-packed vec4.
		static_assert(sizeof(GpuParticle) == 80, "GpuParticle must be 5 tightly-packed vec4 (80 bytes); check Vec4f size/padding.");

		if (m_glReady)
			return;

		// Zeroed state => every slot has life == 0 => culled until a spawn overwrites
		// it. memset rather than value-init: Vec4f's default ctor may leave garbage.
		std::vector<GpuParticle> zero(m_capacity);
		std::memset(zero.data(), 0, zero.size() * sizeof(GpuParticle));
		const GLsizeiptr bytes = (GLsizeiptr)m_capacity * (GLsizeiptr)sizeof(GpuParticle);
		const GLsizei stride = (GLsizei)sizeof(GpuParticle);

		glGenBuffers(2, m_vbo);
		glGenVertexArrays(2, m_vao);
		for (int i = 0; i < 2; i++)
		{
			glBindVertexArray(m_vao[i]);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo[i]);
			glBufferData(GL_ARRAY_BUFFER, bytes, zero.data(), GL_DYNAMIC_DRAW);
			for (GLuint a = 0; a < 5; a++)
			{
				glEnableVertexAttribArray(a);
				glVertexAttribPointer(a, 4, GL_FLOAT, GL_FALSE, stride, (const void*)(size_t)(a * sizeof(Vec4f)));
			}
		}
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_front = 0;
		m_cursor = 0;
		m_glReady = true;
	}

	void GpuParticleSystem::ApplySpawns()
	{
		int front = m_spawnCurrent.load();
		auto& spawns = m_spawns[front];
		if (spawns.empty())
			return;

		uint32_t n = (uint32_t)spawns.size();
		if (n > m_capacity)
		{
			// One frame asked for more than the whole pool: keep the most recent.
			spawns.erase(spawns.begin(), spawns.begin() + (n - m_capacity));
			n = m_capacity;
		}

		const GLsizei stride = (GLsizei)sizeof(GpuParticle);

		// Spawns go into the buffer the Update pass will READ this frame (m_front).
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[m_front]);
		uint32_t start = m_cursor;
		uint32_t firstRun = std::min(n, m_capacity - start);
		glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)start * stride, (GLsizeiptr)firstRun * stride, spawns.data());
		if (n > firstRun)
			glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(n - firstRun) * stride, spawns.data() + firstRun);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_cursor = (start + n) % m_capacity;
		spawns.clear();
	}

	void GpuParticleSystem::Update(const GpuParticleShaders& shaders, float dt)
	{
		if (!m_glReady)
			return;

		int back = m_front ^ 1;

		shaders.update->Bind();
		shaders.update->SetUniform(shaders.uDt, (GLfloat)dt);
		shaders.update->SetUniform(shaders.uGravity, m_gravity);
		shaders.update->SetUniform(shaders.uDrag, (GLfloat)m_drag);

		glEnable(GL_RASTERIZER_DISCARD);
		glBindVertexArray(m_vao[m_front]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_vbo[back]);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, (GLsizei)m_capacity);
		glEndTransformFeedback();
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
		glBindVertexArray(0);
		glDisable(GL_RASTERIZER_DISCARD);

		m_front = back; // freshly-updated state is now the front buffer
	}

	void GpuParticleSystem::Render(const GpuParticleShaders& shaders, sptr<RenderTarget> target)
	{
		if (!m_glReady)
			return;

		target->Bind();

		// Blend into the layer FBO. The fragment colour is straight (texture*tint),
		// not premultiplied, so the source-colour factors below behave as expected.
		switch (m_blend)
		{
		case GpuParticleBlend::Additive:
			// Light sums; RGB clamps at 1 -> dense overlap goes white. Alpha
			// accumulates as coverage so the glow shows through the layer composite.
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE);
			break;
		case GpuParticleBlend::SoftAdditive:
			// Same coverage accumulation, but RGB uses ONE_MINUS_SRC_COLOR so each
			// new contribution attenuates the already-bright destination -> the sum
			// rolls off toward white instead of slamming into it, keeping fire's hue
			// in dense regions.
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE);
			break;
		case GpuParticleBlend::Alpha:
		default:
			// Straight alpha (over): matches the quad pipeline; particles occlude
			// rather than sum, so no glow and no white-out.
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}

		shaders.render->Bind();
		shaders.render->SetUniform(shaders.uAtlas, m_atlas);
		shaders.render->SetUniform(shaders.uTexture, (int)0);
		shaders.render->SetUniform(shaders.uSizeEase, (int)m_sizeEase);
		shaders.render->SetUniform(shaders.uJitterAmp, (GLfloat)m_jitterAmp);
		shaders.render->SetUniform(shaders.uJitterFreq, (GLfloat)m_jitterFreq);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		glBindVertexArray(m_vao[m_front]);
		glDrawArrays(GL_POINTS, 0, (GLsizei)m_capacity);
		glBindVertexArray(0);
	}

	void GpuParticleSystem::DestroyGL()
	{
		if (!m_glReady)
			return;
		glDeleteVertexArrays(2, m_vao);
		glDeleteBuffers(2, m_vbo);
		m_vao[0] = m_vao[1] = 0;
		m_vbo[0] = m_vbo[1] = 0;
		m_glReady = false;
	}
}
