#pragma once

#include "box2d/box2d.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Physics/Collider.h"
#include "SandCastle/Core/Bitmask.h"
#include "SandCastle/ECS/EntityId.h"
#include "SandCastle/ECS/Components.h"
#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	class PhysicsSystem;

	/// @brief Output of Physics::RaycastClosest / RaycastAll. `hit` indicates whether the ray hit anything; the bool conversion lets `if (result)` work directly.
	struct RaycastResult
	{
		EntityId entityId = EntityId(0);
		Vec2f point = 0;
		Vec2f normal = 0;
		float distance = 0;
		bool hit = false;
		Bitmask16 layer;

		operator bool()
		{
			return hit;
		}
	};

	/// @brief Output of overlap queries (CircleOverlap, BodyOverlap, PointInside): identifies the entity, optional distance, and the layer it sits on.
	struct OverlapResult
	{
		EntityId entityId;
		float distance;
		Bitmask16 layer;
	};

	/// @brief Wrapper around a Box2D body. Add colliders (Box2D, Circle2D, Polygon2D) to it.
	/// Use the StaticBody / KinematicBody / DynamicBody subclasses to actually create one:
	/// - StaticBody sits at a fixed position.
	/// - KinematicBody follows the entity's Transform (synced by PhysicsSystem).
	/// - DynamicBody is simulated by Box2D and drives the entity's Transform.
	/// Set the collision layer/mask any time; live shapes are updated.
	struct Body
	{
	public:
		/// The Box2D side keeps a pointer to `userData`, addresses must be stable.
		PointableComponent;

		/// @brief Matches Box2D body types. Each has a dedicated subclass.
		enum class Type
		{
			Static, Kinematic, Dynamic
		};

		Body(Bitmask16 layer = 1);
		Body(Body&& body) noexcept;
		Body& operator=(Body&& body) noexcept;
		virtual ~Body();
		/// @brief Set the layers it's on, shapes already created are updated.
		/// @param layer
		void SetLayer(Bitmask16 layer);
		/// @brief Set the layers it collide with, shapes already created are updated.
		/// @param mask
		void SetLayerMask(Bitmask16 mask);
		/// @brief Add a collider.
		/// Warning: collider geometry cannot be changed after being added. Make sure it's properly initialized
		/// (i.e : Polygon2D SetPoints before adding )
		/// @param collider
		void AddCollider(sptr<Collider> collider);
		/// @brief Remove all the colliders from the body.
		void ClearCollider();

		/// @brief Get the list of overlapping bodies within the appropriate collision layer
		/// @param results
		void OverlappingBodies(std::vector<OverlapResult>& results);
		/// @brief Teleport the body. Rotation in degrees, counter-clockwise positive (matches the renderer).
		virtual void UpdateTransform(Vec3f position, float rotation);
		/// @brief Set true for collision to happen in the X/Z plane
		/// @param yIsZ
		void SetYisZ(bool yIsZ);

		/// @brief World position of the body, straight from Box2D.
		Vec2f GetPosition() const;
		/// @brief World rotation of the body in degrees, counter-clockwise positive.
		float GetRotation() const;
		/// @brief Bits identifying which layer(s) this body occupies.
		Bitmask16 GetLayer() const;
		/// @brief Bits identifying which layers this body collides against.
		Bitmask16 GetLayerMask() const;
		/// @brief Underlying Box2D body id (advanced usage: forces, joints, etc.).
		b2BodyId GetB2Body() const;
		/// @brief All colliders attached to this body.
		const std::vector<sptr<Collider>>* GetColliders();
		/// @brief Combined AABB of every collider on this body.
		b2AABB GetAABB();

	public:
		/// @brief Used internally to identify the entity owning this body within Box2D queries.
		Collider::UserData userData;

	protected:

		friend PhysicsSystem;
		b2Filter GetB2Filter() const;
		/// @brief Create the underlying Box2D body, bound to this instance. Called by subclasses.
		void CreateB2Body(b2BodyType type, Vec2f position, Bitmask16 layer);
		/// @brief Steal the Box2D body of another instance (move construction).
		void MoveFrom(Body&& body) noexcept;
		/// @brief Unbind every collider (their shapes die with the Box2D body).
		void DetachColliders();

		std::vector<sptr<Collider>> m_colliders;
		Bitmask16 m_layer;
		Bitmask16 m_mask;
		b2BodyId m_bodyId;
		/// Last rotation set through UpdateTransform, in degrees. Avoids
		/// redundant (expensive) Box2D teleports when nothing moved.
		float m_rotation;

		bool m_YisZ;
	};
	/// @brief Will not move with the transform
	class StaticBody : public Body
	{
	public:
		StaticBody(Vec2f position, Bitmask16 layer = 1);
		StaticBody(StaticBody&& body) noexcept;
		StaticBody& operator=(StaticBody&& body) noexcept = default;
	};
	/// @brief Automatically update position with transform component thanks to PhysicsSystem
	class KinematicBody : public Body
	{
	public:
		KinematicBody(Bitmask16 layer = 1);
		KinematicBody(KinematicBody&& body) noexcept;
		KinematicBody& operator=(KinematicBody&& body) noexcept = default;
	};
	/// @brief Fully simulated by Box2D: gravity, forces, impulses, collision response.
	/// The PhysicsSystem steps the world on the fixed timestep and writes the simulated
	/// state back to the entity's Transform, interpolated between the last two fixed
	/// steps so visuals stay smooth at any framerate (at the cost of up to one fixed
	/// step of latency). Don't parent the transform and don't move it manually: the
	/// simulation owns it.
	/// Mass comes from the colliders (density 1). Colliders default friction: 0.6, restitution: 0.
	class DynamicBody : public Body
	{
	public:
		DynamicBody(Vec2f position, Bitmask16 layer = 1);
		DynamicBody(DynamicBody&& body) noexcept;
		DynamicBody& operator=(DynamicBody&& body) noexcept = default;

		/// @brief Teleport the body and reset the visual interpolation to the new state
		/// (no glide from the old position).
		void UpdateTransform(Vec3f position, float rotation) override;
		/// @brief Set the linear velocity, in world units per second. Wakes the body.
		void SetVelocity(Vec2f velocity);
		Vec2f GetVelocity() const;
		/// @brief Set the angular velocity, in degrees per second, counter-clockwise positive. Wakes the body.
		void SetAngularVelocity(float degreesPerSecond);
		float GetAngularVelocity() const;
		/// @brief Apply a force at the center of mass, accumulated until the next step. Wakes the body.
		void ApplyForce(Vec2f force);
		/// @brief Apply an instant impulse at the center of mass (velocity change = impulse / mass). Wakes the body.
		void ApplyImpulse(Vec2f impulse);
		/// @brief Per-body gravity multiplier: 0 floats, 1 default, negative rises. Wakes the body.
		void SetGravityScale(float scale);
		float GetGravityScale() const;
		/// @brief Prevent the simulation from rotating this body.
		void SetFixedRotation(bool fixed);
		/// @brief Velocity decay per second (0 = none). Acts like drag.
		void SetLinearDamping(float damping);
		/// @brief Angular velocity decay per second (0 = none).
		void SetAngularDamping(float damping);
		/// @brief Mass computed from the colliders.
		float GetMass() const;

	protected:
		friend PhysicsSystem;
		//Simulated state snapshots taken at each fixed step; the PhysicsSystem
		//interpolates the entity's Transform between them every frame.
		Vec2f m_prevPosition;
		Vec2f m_currPosition;
		float m_prevRotation = 0.f;
		float m_currRotation = 0.f;
	};
}
