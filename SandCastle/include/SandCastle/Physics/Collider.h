#pragma once

#include <vector>
#include <earcut/earcut.hpp>
#include <box2D/box2d.h>
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

	struct Triangle
	{
		Vec2f a;
		Vec2f b;
		Vec2f c;
	};

	struct ColliderRender;

	/// @brief Interface class for colliders, need to be added to a Body
	/// Can be any type of shape.
	class Collider
	{
	public:
		struct UserData
		{
			EntityId entityId = EntityId(0);
		};
		virtual ~Collider() {}
		virtual bool B2ShapeOverlap(b2Shape* shape, b2Transform& transform) = 0;
		virtual bool ColliderOverlap(Collider* collider) = 0;
		virtual bool CircleOverlap(Vec2f point, float radius) = 0;
		virtual bool PointInside(Vec2f point) = 0;
		virtual void SetupRender(ColliderRender* render) = 0;
		virtual b2AABB GetAABB() = 0;
		
		Body* GetBody()
		{
			return m_body;
		}
	protected:
		friend Body;
		virtual void SetBody(Body* body, b2Filter filter, UserData& userData) = 0;
		Body* m_body = nullptr;
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

	/// @brief Wrapper around a fixturedef
	class Box2D : public Collider
	{

	public:
		/// @brief Create a box of dimension 1x1;
		Box2D();
		Box2D(Vec2f dimensions);
		Box2D(float width, float height);

		void SetBody(Body* body, b2Filter filter, UserData& userData) override;

		bool B2ShapeOverlap(b2Shape* shape, b2Transform& transform) override;
		bool ColliderOverlap(Collider* collider) override;
		bool CircleOverlap(Vec2f point, float radius) override;
		bool PointInside(Vec2f point) override;
		void SetupRender(ColliderRender* render) override;
		virtual b2AABB GetAABB() override;
	

	private:
		b2PolygonShape m_shape;
	};

	class Circle2D : public Collider
	{
		
	public:
		/// @brief Create circle of radius 1
		Circle2D();
		Circle2D(float radius);


		void SetBody(Body* body, b2Filter filter, UserData& userData) override;

		bool B2ShapeOverlap(b2Shape* shape, b2Transform& transform) override;
		bool ColliderOverlap(Collider* collider) override;
		bool CircleOverlap(Vec2f point, float radius) override;
		bool PointInside(Vec2f point) override;
		void SetupRender(ColliderRender* render) override;
		b2AABB GetAABB() override;

	private:
		b2CircleShape m_shape;

	};

	/// @brief Not implemented
	class Polygon2D : public Collider
	{
	public:
		Polygon2D();
		Polygon2D(std::vector<Vec2f> points);

		void SetBody(Body* body, b2Filter filter, UserData& userData) override;

		bool B2ShapeOverlap(b2Shape* shape, b2Transform& transform) override;
		bool ColliderOverlap(Collider* collider) override;
		bool CircleOverlap(Vec2f point, float radius) override;
		bool PointInside(Vec2f point) override;
		void SetupRender(ColliderRender* render) override;
		b2AABB GetAABB() override;

		/// @brief Counter Clockwise Winding order
		/// @param point 
		void AddPoint(Vec2f point);
		void SetPoints(std::vector<Vec2f>& points);



	private:
		void BakeTriangles();
		std::vector<b2PolygonShape> m_shapes;
		std::vector<uint32_t> m_triangles;
		std::vector<std::vector<Vec2f>> m_points;
	};
}
