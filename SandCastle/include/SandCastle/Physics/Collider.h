#pragma once

#include <vector>
#include <earcut/earcut.hpp>
#include <box2d/box2d.h>
#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/EntityId.h"
#include "SandCastle/Render/WireRender.h"

namespace mapbox {
	namespace util {

		template <>
		struct nth<0, SandCastle::Vec3f> {
			inline static auto get(const SandCastle::Vec3f& t) {
				return t.x;
			};
		};
		template <>
		struct nth<1, SandCastle::Vec3f> {
			inline static auto get(const SandCastle::Vec3f& t) {
				return t.y;
			};
		};

		template <>
		struct nth<0, SandCastle::Vec2f> {
			inline static auto get(const SandCastle::Vec2f& t) {
				return t.x;
			};
		};
		template <>
		struct nth<1, SandCastle::Vec2f> {
			inline static auto get(const SandCastle::Vec2f& t) {
				return t.y;
			};
		};
	}
}

namespace SandCastle
{
	struct Body;
	struct ColliderRender;

	/// @brief For internal use: build a GJK proxy from any live Box2D shape (circle, capsule, segment or polygon).
	b2ShapeProxy GetShapeProxy(b2ShapeId shapeId);

	/// @brief Interface class for colliders, need to be added to a Body.
	/// Concrete shapes: Box2D, Circle2D, Polygon2D. Geometry is fixed once
	/// the collider has been added to a Body.
	class Collider
	{
	public:
		struct UserData
		{
			EntityId entityId = EntityId(0);
		};
		virtual ~Collider() {}

		/// @brief For internal use: true if any of this collider's shapes overlaps the given proxy (world space).
		bool ProxyOverlap(const b2ShapeProxy& proxy, b2Transform proxyTransform);
		/// @brief True if any shape of `collider` overlaps any shape of this collider. Both must be attached to a Body.
		bool ColliderOverlap(Collider* collider);
		/// @brief True if the given world-space circle overlaps this collider. Must be attached to a Body.
		bool CircleOverlap(Vec2f point, float radius);
		/// @brief True if the given world-space point is inside this collider. Must be attached to a Body.
		bool PointInside(Vec2f point);
		/// @brief Combined world AABB of every Box2D shape created by this collider.
		b2AABB GetAABB();
		/// @brief Build the debug wireframe for this collider.
		virtual void SetupRender(ColliderRender* render) = 0;

		/// @brief The Body this collider is attached to, nullptr if none.
		Body* GetBody()
		{
			return m_body;
		}
	protected:
		friend Body;
		/// @brief Create the Box2D shapes on the body. Called once by Body::AddCollider.
		virtual void CreateShapes(const b2ShapeDef& def) = 0;
		void SetBody(Body* body, const b2Filter& filter);
		void SetFilter(const b2Filter& filter);
		void DestroyShapes();

		Body* m_body = nullptr;
		std::vector<b2ShapeId> m_shapes;
	};


	struct ColliderRender
	{

		ColliderRender(ColliderRender&& collider)  noexcept :
			wire(collider.wire)
		{

		}
		ColliderRender(Collider* collider)
		{
			collider->SetupRender(this);
		}
		sptr<WireRender> wire;
	};

	/// @brief Axis-aligned box collider (Box2D polygon under the hood).
	class Box2D : public Collider
	{

	public:
		/// @brief Create a box of dimension 1x1
		Box2D();
		Box2D(Vec2f dimensions);
		Box2D(float width, float height);

		void SetupRender(ColliderRender* render) override;

	protected:
		void CreateShapes(const b2ShapeDef& def) override;

	private:
		b2Polygon m_polygon;
	};

	/// @brief Circle collider.
	class Circle2D : public Collider
	{

	public:
		/// @brief Create circle of radius 1
		Circle2D();
		Circle2D(float radius);

		void SetupRender(ColliderRender* render) override;

	protected:
		void CreateShapes(const b2ShapeDef& def) override;

	private:
		b2Circle m_circle;

	};

	/// @brief Concave polygon collider. The outline is triangulated (earcut) into
	/// convex Box2D polygons when the collider is added to a Body.
	class Polygon2D : public Collider
	{
	public:
		Polygon2D();
		Polygon2D(std::vector<Vec2f> points);

		void SetupRender(ColliderRender* render) override;

		/// @brief Counter Clockwise Winding order
		/// @param point
		void AddPoint(Vec2f point);
		void SetPoints(std::vector<Vec2f>& points);

	protected:
		void CreateShapes(const b2ShapeDef& def) override;

	private:
		std::vector<uint32_t> BakeTriangles();
		std::vector<std::vector<Vec2f>> m_points;
	};
}
