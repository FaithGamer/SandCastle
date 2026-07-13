#pragma once

#include <SandCastle.h>
#include <vector>
#include <cmath>

/*

Physics performance / limitations bench.

A big U-shaped bucket (three kinematic walls) spans almost the whole screen,
and an emitter at the cursor rains tiny dynamic shapes into it. Everything is
tuned live from an ImGui window to find where the simulation breaks down:
body count, solver sub-steps, sleeping, continuous collision, restitution...

- A/D: rotate the bucket (kinematic teleport, spills the shapes around).
- ImGui "Physics bench" window:
	* emit rate (shapes per second, spawned at the cursor)
	* emitted shape: circle or box, and its size        (not retroactive)
	* material: density, friction, restitution          (not retroactive)
	* body: gravity scale, linear damping, fixed rotation, bullet (CCD)
														(not retroactive)
	* world: gravity, solver sub-steps, sleeping, continuous collision
	* live counters: bodies, contacts, islands, fps/frame time
	* clear all emitted shapes (the bucket stays)

Interesting limits to look for: stack jitter at low sub-steps, tiny shapes
tunneling through the walls when continuous collision is off (or when the
bucket rotates fast), fps drop past a few thousand awake bodies, sleeping
disabled keeping every island active forever.

*/

namespace PhysicsBenchImpl
{
	using namespace SandCastle;

	class BenchSystem : public System
	{
	public:
		BenchSystem()
		{
			//Before the PhysicsSystem (99999) so transforms set here are synced this frame
			SetPriority(100000);
		}

		void Start() override
		{
			LOG_INFO("--Physics bench--");
			LOG_INFO("A/D: rotate the bucket | everything else: ImGui window");

			//Size the bucket from the actual visible world area
			Vec2i screen = Window::GetSize();
			Vec3f topLeft = Camera::main->ScreenToWorld(Vec2f(0, 0), Window::GetSize());
			Vec3f bottomRight = Camera::main->ScreenToWorld(Vec2f((float)screen.x, (float)screen.y), Window::GetSize());
			m_bucketCenter = Vec2f((topLeft.x + bottomRight.x) * 0.5f, (topLeft.y + bottomRight.y) * 0.5f);
			float width = (bottomRight.x - topLeft.x) * 0.92f;
			float height = (topLeft.y - bottomRight.y) * 0.92f;

			CreateBucket(width, height, 3.f);

			auto map = Inputs::GetInputMap("bench");
			map->GetInput("Rotate")->signal.Listen(&BenchSystem::OnRotate, this);
		}

		void Update() override
		{
			//Apply world settings from the ImGui thread on the logic thread
			ApplyWorldSettings();

			if (m_rotateDir != 0.f)
			{
				m_bucketAngle += m_rotateDir * 45.f * Time::Delta();
				UpdateBucket();
			}

			if (m_clearQueued)
			{
				m_clearQueued = false;
				ClearShapes();
			}

			//Emission, rate-based; cap the backlog so a frame spike doesn't
			//spawn thousands at once and spiral the framerate down
			m_emitAccumulator += m_rate * Time::Delta();
			if (m_emitAccumulator > 100.f)
			{
				m_emitAccumulator = 100.f;
			}
			Vec3f mouse = Mouse::GetWorldPos();
			while (m_emitAccumulator >= 1.f)
			{
				m_emitAccumulator -= 1.f;
				Emit(Vec2f(mouse.x, mouse.y));
			}

			//Cache the counters here: OnImGui runs on the render thread and
			//must not call into the Box2D world
			b2Counters counters = b2World_GetCounters(Physics::GetB2World());
			m_statBodies = counters.bodyCount;
			m_statContacts = counters.contactCount;
			m_statIslands = counters.islandCount;
		}

		void OnImGui() override
		{
			//Runs on the render thread: only writes plain values read by Update,
			//and queues anything that must touch the world or the registry
			ImGui::Begin("Physics bench");

			ImGui::Text("bodies: %d (emitted: %d)", m_statBodies, (int)m_emittedCount);
			ImGui::Text("contacts: %d | islands: %d", m_statContacts, m_statIslands);
			ImGui::Text("%.1f fps (%.2f ms)", ImGui::GetIO().Framerate, 1000.f / ImGui::GetIO().Framerate);

			ImGui::SeparatorText("Emitter");
			ImGui::SliderFloat("rate (shapes/s)", &m_rate, 0.f, 1000.f);
			ImGui::RadioButton("circle", &m_emitShape, 0);
			ImGui::SameLine();
			ImGui::RadioButton("box", &m_emitShape, 1);
			if (ImGui::Button("clear emitted shapes"))
			{
				m_clearQueued = true;
			}

			ImGui::SeparatorText("New shapes (not retroactive)");
			ImGui::SliderFloat("size", &m_size, 0.2f, 5.f);
			ImGui::SliderFloat("density", &m_density, 0.1f, 10.f);
			ImGui::SliderFloat("friction", &m_friction, 0.f, 1.f);
			ImGui::SliderFloat("restitution", &m_restitution, 0.f, 1.f);
			ImGui::SliderFloat("gravity scale", &m_gravityScale, -2.f, 2.f);
			ImGui::SliderFloat("linear damping", &m_linearDamping, 0.f, 5.f);
			ImGui::Checkbox("fixed rotation", &m_fixedRotation);
			ImGui::SameLine();
			ImGui::Checkbox("bullet (CCD)", &m_bullet);

			ImGui::SeparatorText("World");
			ImGui::SliderFloat("gravity y", &m_gravityY, -200.f, 50.f);
			ImGui::SliderInt("sub-steps", &m_subSteps, 1, 16);
			ImGui::Checkbox("sleeping", &m_sleeping);
			ImGui::SameLine();
			ImGui::Checkbox("continuous collision", &m_continuous);

			ImGui::End();
		}

		int GetUsedMethod() override
		{
			return System::Method::Updt | System::Method::ImGui;
		}

	private:

		struct BucketPart
		{
			Entity entity;
			Vec2f offset;
		};

		//Counter-clockwise rotation, matching the engine convention
		static Vec2f Rotate(Vec2f v, float degrees)
		{
			float radians = Math::Radians(degrees);
			float c = cosf(radians);
			float s = sinf(radians);
			return Vec2f(c * v.x - s * v.y, s * v.x + c * v.y);
		}

		void CreateBucket(float width, float height, float thickness)
		{
			//Floor and two walls, open at the top
			Vec2f offsets[3] = {
				Vec2f(0, -height * 0.5f + thickness * 0.5f),
				Vec2f(-width * 0.5f + thickness * 0.5f, 0),
				Vec2f(width * 0.5f - thickness * 0.5f, 0) };
			Vec2f sizes[3] = {
				Vec2f(width, thickness),
				Vec2f(thickness, height),
				Vec2f(thickness, height) };

			for (int i = 0; i < 3; i++)
			{
				Entity entity = Entity::CreateSprite("square.png_0_0");
				entity.Get<Transform>()->SetScale(sizes[i]);
				entity.Get<SpriteRender>()->color = Color(120, 120, 135, 255);

				//Kinematic: rotating the bucket goes through the Transform,
				//the PhysicsSystem teleports the body to match
				auto* body = entity.AddGet<KinematicBody>();
				body->userData.entityId = entity.GetId();
				body->AddCollider(makesptr<Box2D>(sizes[i].x, sizes[i].y));

				m_bucket[i] = BucketPart{ entity, offsets[i] };
			}
			UpdateBucket();
		}

		void UpdateBucket()
		{
			for (auto& part : m_bucket)
			{
				Vec2f position = m_bucketCenter + Rotate(part.offset, m_bucketAngle);
				auto* transform = part.entity.Get<Transform>();
				transform->SetPosition(position.x, position.y, 0);
				transform->SetRotation(m_bucketAngle);
			}
		}

		void Emit(Vec2f position)
		{
			static const Color palette[5] = {
				Color(230, 200, 120, 255), Color(220, 140, 160, 255),
				Color(150, 210, 210, 255), Color(200, 220, 140, 255),
				Color(170, 160, 230, 255) };

			//Tiny jitter so same-frame spawns don't overlap on the exact same point
			position.x += Random::Range(-0.2f, 0.2f);
			position.y += Random::Range(-0.2f, 0.2f);

			bool box = m_emitShape == 1;
			float size = m_size;
			Entity entity = Entity::CreateSprite(box ? "square.png_0_0" : "circle.png_0_0");
			auto* transform = entity.Get<Transform>();
			transform->SetPosition(position.x, position.y, 0);
			transform->SetScale(size, size);
			entity.Get<SpriteRender>()->color = palette[m_emittedCount % 5];

			auto* body = entity.AddGet<DynamicBody>(position);
			body->userData.entityId = entity.GetId();

			sptr<Collider> collider;
			if (box)
			{
				collider = makesptr<Box2D>(size, size);
			}
			else
			{
				collider = makesptr<Circle2D>(size * 0.5f);
			}
			collider->SetMaterial(m_density, m_friction, m_restitution);
			body->AddCollider(collider);

			body->SetGravityScale(m_gravityScale);
			body->SetLinearDamping(m_linearDamping);
			body->SetFixedRotation(m_fixedRotation);
			if (m_bullet)
			{
				b2Body_SetBullet(body->GetB2Body(), true);
			}

			m_shapes.emplace_back(entity);
			m_emittedCount++;
		}

		void ClearShapes()
		{
			for (auto& entity : m_shapes)
			{
				entity.Destroy();
			}
			m_shapes.clear();
			m_emittedCount = 0;
			LOG_INFO("Cleared, body count: {0}", Physics::GetBodyCount());
		}

		void ApplyWorldSettings()
		{
			if (m_gravityY != m_appliedGravityY)
			{
				m_appliedGravityY = m_gravityY;
				Physics::SetGravity(Vec2f(0, m_gravityY));
			}
			if (m_subSteps != Physics::GetSubStepCount())
			{
				Physics::SetSubStepCount(m_subSteps);
			}
			if (m_sleeping != m_appliedSleeping)
			{
				m_appliedSleeping = m_sleeping;
				b2World_EnableSleeping(Physics::GetB2World(), m_sleeping);
			}
			if (m_continuous != m_appliedContinuous)
			{
				m_appliedContinuous = m_continuous;
				b2World_EnableContinuous(Physics::GetB2World(), m_continuous);
			}
		}

		void OnRotate(InputSignal* signal)
		{
			m_rotateDir = signal->GetVec2f().x;
		}

	private:
		BucketPart m_bucket[3];
		std::vector<Entity> m_shapes;
		Vec2f m_bucketCenter = Vec2f(0, 0);
		float m_bucketAngle = 0.f;
		float m_rotateDir = 0.f;
		float m_emitAccumulator = 0.f;
		size_t m_emittedCount = 0;
		bool m_clearQueued = false;
		int m_statBodies = 0;
		int m_statContacts = 0;
		int m_statIslands = 0;

		//Emitter settings (written by the ImGui thread, read here; plain
		//values only, torn reads are harmless for a bench)
		float m_rate = 50.f;
		int m_emitShape = 0; //0 circle, 1 box
		float m_size = 1.f;
		float m_density = 1.f;
		float m_friction = 0.6f;
		float m_restitution = 0.f;
		float m_gravityScale = 1.f;
		float m_linearDamping = 0.f;
		bool m_fixedRotation = false;
		bool m_bullet = false;

		//World settings, applied on the logic thread in Update
		float m_gravityY = -40.f;
		float m_appliedGravityY = 0.f;
		int m_subSteps = 4;
		bool m_sleeping = true;
		bool m_appliedSleeping = true;
		bool m_continuous = true;
		bool m_appliedContinuous = true;
	};
}

inline void PhysicsBench()
{
	using namespace SandCastle;

	Engine::Init();

	auto map = Inputs::CreateInputMap("bench");
	auto rotate = map->CreateDirectionalInput("Rotate");
	std::vector<DirectionalButton> buttons;
	buttons.emplace_back(DirectionalButton(Button(Key::Scancode::A), Vec2f(1.f, 0)));
	buttons.emplace_back(DirectionalButton(Button(Key::Scancode::D), Vec2f(-1.f, 0)));
	rotate->BindButtons(buttons);

	Systems::Push<PhysicsBenchImpl::BenchSystem>();

	Engine::Launch();
}
