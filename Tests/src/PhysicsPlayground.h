#pragma once

#include <SandCastle.h>
#include <vector>

/*

Interactive physics playground: visuals are built from simple tinted shapes
(1px white square + white circle sprites) scaled to match the colliders
exactly, so what you see is what the physics sees.

- Move the mouse: the translucent player shape follows it (KinematicBody
  synced from Transform by the PhysicsSystem). Bodies it overlaps turn red,
  the player turns orange (Body::OverlappingBodies against box, rotated box,
  circle, concave polygon).
- The body directly under the cursor turns cyan (Physics::PointInside):
  the highlight must flip exactly on the visual edge of every shape.
- A ray is cast continuously from the emitter at the top toward the mouse
  (Physics::RaycastClosest). The beam sprite is stretched to the reported
  hit distance, so it must visually stop exactly on the surface it hits.
  The hit body turns yellow. The ray is masked to layer 1: it passes
  through the player (layer 2).
- Left click: place a static copy of the player's current shape, with its
  current rotation.
- D: drop a dynamic copy of the player's current shape. It falls under
  gravity, collides with the scene and the other dynamic shapes, and its
  sprite follows the simulation (DynamicBody, Transform written back by the
  PhysicsSystem each fixed step). The kinematic player can shove it around.
- Right click: remove the last placed shape.
- Space: toggle the player collider between circle and box
  (ClearCollider + AddCollider at runtime).
- Q / E: rotate the player (visible in box mode, rotation reaches the
  collider through the PhysicsSystem sync and is applied to placed shapes).

*/

namespace PhysicsPlaygroundImpl
{
	using namespace SandCastle;

	//Marks a body entity of the playground. Tracks which entities carry its
	//sprites (a concave polygon body is displayed by several child sprites).
	struct PlaygroundBody
	{
		Color baseColor;
		std::vector<EntityId> visuals;
	};

	class PlaygroundSystem : public System
	{
	public:
		PlaygroundSystem()
		{
			//Run before the PhysicsSystem (99999) so bodies are synced with
			//this frame's transforms before we query them.
			SetPriority(100000);
		}

		void Start() override
		{
			LOG_INFO("--Physics playground--");
			LOG_INFO("Mouse: move the player | Left click: place current shape | Right click: remove last placed");
			LOG_INFO("Space: toggle player shape circle/box | Q/E: rotate the player | D: drop a dynamic shape");
			LOG_INFO("Red: overlaps the player | Cyan: under the cursor | Yellow: hit by the ray");

			//Player (z=2) above the ray beam (z=1) above the scene (z=0)
			Renderer2D::SetLayerSortZ(SpriteRender::defaultLayer, true);

			CreateScene();
			CreatePlayer();
			CreateRay();

			auto map = Inputs::GetInputMap("playground");
			map->GetInput("Spawn")->signal.Listen(&PlaygroundSystem::OnSpawn, this);
			map->GetInput("Despawn")->signal.Listen(&PlaygroundSystem::OnDespawn, this);
			map->GetInput("ToggleShape")->signal.Listen(&PlaygroundSystem::OnToggleShape, this);
			map->GetInput("Rotate")->signal.Listen(&PlaygroundSystem::OnRotate, this);
			map->GetInput("Drop")->signal.Listen(&PlaygroundSystem::OnDrop, this);
		}

		void Update() override
		{
			Vec3f mouse = Mouse::GetWorldPos();

			//The PhysicsSystem propagates this to the KinematicBody
			auto* transform = m_player.Get<Transform>();
			transform->SetPosition(mouse.x, mouse.y, 2);
			if (m_rotateDir != 0.f)
			{
				transform->Rotate(m_rotateDir * 120.f * Time::Delta());
			}

			if (m_toggleQueued)
			{
				m_toggleQueued = false;
				TogglePlayerShape();
			}
			if (m_spawnQueued)
			{
				m_spawnQueued = false;
				SpawnShape(mouse, transform->GetRotation());
			}
			if (m_dropQueued)
			{
				m_dropQueued = false;
				SpawnDynamic(mouse, transform->GetRotation());
			}
			if (m_despawnQueued)
			{
				m_despawnQueued = false;
				DespawnBox();
			}

			//Repaint every body with its base color
			Entity::View<PlaygroundBody>().each([](PlaygroundBody& body)
				{
					SetVisualsColor(body, body.baseColor);
				});

			//Bodies overlapping the player shape turn red, the player turns orange
			auto* playerBody = m_player.Get<KinematicBody>();
			std::vector<OverlapResult> overlaps;
			playerBody->OverlappingBodies(overlaps);
			for (auto& overlap : overlaps)
			{
				TintBody(overlap.entityId, Color(235, 70, 70, 255));
			}
			m_player.Get<SpriteRender>()->color =
				overlaps.empty() ? Color(240, 240, 240, 150) : Color(255, 160, 60, 150);

			UpdateRay(mouse);

			//The body directly under the cursor turns cyan (Physics::PointInside):
			//the highlight must switch exactly when the cursor crosses the visual edge
			std::vector<OverlapResult> underCursor;
			Physics::PointInside(underCursor, Vec2f(mouse.x, mouse.y), Bitmask16(1));
			for (auto& result : underCursor)
			{
				TintBody(result.entityId, Color(90, 220, 235, 255));
			}
		}

		int GetUsedMethod() override
		{
			return System::Method::Updt;
		}

	private:

		//
		// Scene construction
		//

		static void SetVisualsColor(PlaygroundBody& body, Color color)
		{
			for (auto id : body.visuals)
			{
				Entity visual(id);
				if (visual.Valid())
				{
					visual.Get<SpriteRender>()->color = color;
				}
			}
		}

		static void TintBody(EntityId id, Color color)
		{
			Entity entity(id);
			if (!entity.Valid())
				return;
			auto* body = entity.Get<PlaygroundBody>();
			if (body != nullptr)
			{
				SetVisualsColor(*body, color);
			}
		}

		static Entity MakeBoxBody(Vec2f position, float width, float height, float rotation, Color color)
		{
			Entity entity = Entity::CreateSprite("square.png_0_0");
			auto* transform = entity.Get<Transform>();
			transform->SetPosition(position.x, position.y, 0);
			transform->SetScale(width, height);
			transform->SetRotation(rotation);

			auto* body = entity.AddGet<StaticBody>(position);
			body->userData.entityId = entity.GetId();
			body->AddCollider(makesptr<Box2D>(width, height));
			if (rotation != 0.f)
			{
				body->UpdateTransform(Vec3f(position.x, position.y, 0), rotation);
			}

			auto* playground = entity.AddGet<PlaygroundBody>();
			playground->baseColor = color;
			playground->visuals = { entity.GetId() };
			entity.Get<SpriteRender>()->color = color;
			return entity;
		}

		static Entity MakeCircleBody(Vec2f position, float radius, Color color)
		{
			Entity entity = Entity::CreateSprite("circle.png_0_0");
			auto* transform = entity.Get<Transform>();
			transform->SetPosition(position.x, position.y, 0);
			transform->SetScale(radius * 2.f, radius * 2.f);

			auto* body = entity.AddGet<StaticBody>(position);
			body->userData.entityId = entity.GetId();
			body->AddCollider(makesptr<Circle2D>(radius));

			auto* playground = entity.AddGet<PlaygroundBody>();
			playground->baseColor = color;
			playground->visuals = { entity.GetId() };
			entity.Get<SpriteRender>()->color = color;
			return entity;
		}

		//L-shaped concave polygon: [0,w]x[0,h] plus [0,h]x[h,w] (w = 2h).
		//One Polygon2D collider, displayed by two child sprites.
		static Entity MakeLBody(Vec2f position, float armLength, float armWidth, Color color)
		{
			Entity entity = Entity::Create();
			entity.Add<Transform>();
			entity.Get<Transform>()->SetPosition(position.x, position.y, 0);

			auto* body = entity.AddGet<StaticBody>(position);
			body->userData.entityId = entity.GetId();

			auto polygon = makesptr<Polygon2D>();
			polygon->AddPoint(Vec2f(0, 0));
			polygon->AddPoint(Vec2f(armLength, 0));
			polygon->AddPoint(Vec2f(armLength, armWidth));
			polygon->AddPoint(Vec2f(armWidth, armWidth));
			polygon->AddPoint(Vec2f(armWidth, armLength));
			polygon->AddPoint(Vec2f(0, armLength));
			body->AddCollider(polygon);

			auto* playground = entity.AddGet<PlaygroundBody>();
			playground->baseColor = color;

			//Horizontal arm then vertical arm
			Vec2f scales[2] = { Vec2f(armLength, armWidth), Vec2f(armWidth, armLength - armWidth) };
			Vec2f offsets[2] = { Vec2f(armLength * 0.5f, armWidth * 0.5f),
								 Vec2f(armWidth * 0.5f, armWidth + (armLength - armWidth) * 0.5f) };
			for (int i = 0; i < 2; i++)
			{
				Entity arm = Entity::CreateSprite("square.png_0_0");
				arm.Get<Transform>()->SetPosition(offsets[i].x, offsets[i].y, 0);
				arm.Get<Transform>()->SetScale(scales[i]);
				arm.Get<SpriteRender>()->color = color;
				entity.AddChild(arm);
				playground->visuals.emplace_back(arm.GetId());
			}
			return entity;
		}

		void CreateScene()
		{
			MakeBoxBody(Vec2f(0, -35), 90.f, 8.f, 0.f, Color(120, 120, 135, 255));   //ground
			MakeBoxBody(Vec2f(-35, -5), 14.f, 14.f, 0.f, Color(90, 140, 255, 255));  //plain box
			MakeBoxBody(Vec2f(35, 0), 18.f, 6.f, 30.f, Color(90, 200, 255, 255));    //rotated box
			MakeCircleBody(Vec2f(-5, 20), 7.f, Color(110, 220, 120, 255));           //circle
			MakeLBody(Vec2f(8, -20), 16.f, 7.f, Color(190, 120, 255, 255));          //concave polygon
		}

		void CreatePlayer()
		{
			m_player = Entity::CreateSprite("circle.png_0_0");
			m_player.Get<Transform>()->SetScale(m_playerSize, m_playerSize);

			//Layer 2: the ray (masked to layer 1) passes through the player
			auto* body = m_player.AddGet<KinematicBody>(Bitmask16(2));
			body->userData.entityId = m_player.GetId();
			body->AddCollider(makesptr<Circle2D>(m_playerSize * 0.5f));
		}

		void CreateRay()
		{
			//Emitter marker
			Entity emitter = Entity::CreateSprite("square.png_0_0");
			emitter.Get<Transform>()->SetPosition(m_rayOrigin.x, m_rayOrigin.y, 1);
			emitter.Get<Transform>()->SetScale(2.f, 2.f);
			emitter.Get<SpriteRender>()->color = Color(255, 240, 150, 255);

			m_ray = Entity::CreateSprite("square.png_0_0");
			m_ray.Get<SpriteRender>()->color = Color(255, 235, 130, 255);
		}

		//
		// Interactions
		//

		void UpdateRay(Vec3f mouse)
		{
			Vec2f toMouse = Vec2f(mouse.x, mouse.y) - m_rayOrigin;
			if (toMouse.Magnitude() < 0.001f)
			{
				toMouse = Vec2f(0, -1);
			}
			Vec2f direction = toMouse.Normalized();
			const float maxLength = 300.f;
			Vec2f end = m_rayOrigin + direction * maxLength;

			//Layer 1 only: the player is on layer 2 and must not block the ray
			RaycastResult hit;
			Physics::RaycastClosest(hit, m_rayOrigin, end, Bitmask16(1));
			float length = hit ? hit.distance : maxLength;
			if (hit)
			{
				TintBody(hit.entityId, Color(255, 235, 90, 255));
			}

			//Stretch the beam sprite from the emitter to the hit point
			Vec2f middle = m_rayOrigin + direction * (length * 0.5f);
			auto* transform = m_ray.Get<Transform>();
			transform->SetPosition(middle.x, middle.y, 1);
			transform->SetScale(0.5f, length);
			transform->SetRotation(Math::VecToAngle(direction));
		}

		void TogglePlayerShape()
		{
			m_playerIsBox = !m_playerIsBox;
			auto* body = m_player.Get<KinematicBody>();
			body->ClearCollider();
			auto* render = m_player.Get<SpriteRender>();
			if (m_playerIsBox)
			{
				body->AddCollider(makesptr<Box2D>(m_playerSize, m_playerSize));
				render->SetSprite(Assets::Get<Sprite>("square.png_0_0"));
			}
			else
			{
				body->AddCollider(makesptr<Circle2D>(m_playerSize * 0.5f));
				render->SetSprite(Assets::Get<Sprite>("circle.png_0_0"));
			}
			LOG_INFO("Player collider: {0}", m_playerIsBox ? "box" : "circle");
		}

		//Place a copy of the player's current shape and rotation
		void SpawnShape(Vec3f position, float rotation)
		{
			static const Color palette[4] = {
				Color(230, 200, 120, 255), Color(220, 140, 160, 255),
				Color(150, 210, 210, 255), Color(200, 220, 140, 255) };
			Color color = palette[m_spawned.size() % 4];
			Vec2f pos(position.x, position.y);
			if (m_playerIsBox)
			{
				m_spawned.emplace_back(MakeBoxBody(pos, m_playerSize, m_playerSize, rotation, color));
			}
			else
			{
				m_spawned.emplace_back(MakeCircleBody(pos, m_playerSize * 0.5f, color));
			}
			LOG_INFO("Spawned {0}, body count: {1}", m_playerIsBox ? "box" : "circle", Physics::GetBodyCount());
		}

		//Drop a simulated copy of the player's current shape: it falls, bounces
		//off the scene, can be shoved by the player, and its sprite follows the
		//simulation through the PhysicsSystem transform write-back
		void SpawnDynamic(Vec3f position, float rotation)
		{
			Vec2f pos(position.x, position.y);
			Entity entity = Entity::CreateSprite(m_playerIsBox ? "square.png_0_0" : "circle.png_0_0");
			auto* transform = entity.Get<Transform>();
			transform->SetPosition(pos.x, pos.y, 0);
			transform->SetScale(m_playerSize, m_playerSize);
			transform->SetRotation(rotation);

			auto* body = entity.AddGet<DynamicBody>(pos);
			body->userData.entityId = entity.GetId();
			if (m_playerIsBox)
			{
				body->AddCollider(makesptr<Box2D>(m_playerSize, m_playerSize));
			}
			else
			{
				body->AddCollider(makesptr<Circle2D>(m_playerSize * 0.5f));
			}
			if (rotation != 0.f)
			{
				body->UpdateTransform(Vec3f(pos.x, pos.y, 0), rotation);
			}

			auto* playground = entity.AddGet<PlaygroundBody>();
			playground->baseColor = Color(245, 245, 245, 255);
			playground->visuals = { entity.GetId() };
			entity.Get<SpriteRender>()->color = playground->baseColor;

			m_spawned.emplace_back(entity);
			LOG_INFO("Dropped dynamic {0}, body count: {1}", m_playerIsBox ? "box" : "circle", Physics::GetBodyCount());
		}

		void DespawnBox()
		{
			if (m_spawned.empty())
				return;
			m_spawned.back().Destroy();
			m_spawned.pop_back();
			LOG_INFO("Removed shape, body count: {0}", Physics::GetBodyCount());
		}

		void OnSpawn(InputSignal* signal)
		{
			m_spawnQueued = true;
		}
		void OnDespawn(InputSignal* signal)
		{
			m_despawnQueued = true;
		}
		void OnToggleShape(InputSignal* signal)
		{
			m_toggleQueued = true;
		}
		void OnRotate(InputSignal* signal)
		{
			m_rotateDir = signal->GetVec2f().x;
		}
		void OnDrop(InputSignal* signal)
		{
			m_dropQueued = true;
		}

	private:
		Entity m_player;
		Entity m_ray;
		std::vector<Entity> m_spawned;
		Vec2f m_rayOrigin = Vec2f(0, 42);
		float m_playerSize = 8.f;
		float m_rotateDir = 0.f;
		bool m_playerIsBox = false;
		bool m_spawnQueued = false;
		bool m_despawnQueued = false;
		bool m_toggleQueued = false;
		bool m_dropQueued = false;
	};
}

inline void PhysicsPlayground()
{
	using namespace SandCastle;

	Engine::Init();

	//Playground shapes are ~8 units wide, so "1 meter" is ~8 units: scale
	//gravity accordingly for a natural falling speed
	Physics::SetGravity(Vec2f(0, -80));

	auto map = Inputs::CreateInputMap("playground");
	map->CreateButtonInput("Spawn")->BindMouse(Mouse::Button::Left);
	map->CreateButtonInput("Despawn")->BindMouse(Mouse::Button::Right);
	map->CreateButtonInput("ToggleShape")->BindKey(Key::Scancode::Space);
	map->CreateButtonInput("Drop")->BindKey(Key::Scancode::D);

	auto rotate = map->CreateDirectionalInput("Rotate");
	std::vector<DirectionalButton> buttons;
	buttons.emplace_back(DirectionalButton(Button(Key::Scancode::Q), Vec2f(-1.f, 0)));
	buttons.emplace_back(DirectionalButton(Button(Key::Scancode::E), Vec2f(1.f, 0)));
	rotate->BindButtons(buttons);

	Systems::Push<PhysicsPlaygroundImpl::PlaygroundSystem>();

	Engine::Launch();
}
