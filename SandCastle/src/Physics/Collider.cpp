#include "pch.h"
#include "SandCastle/Physics/Collider.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Core/Math.h"

namespace SandCastle
{
	//Same tolerance as Box2D's own world overlap queries (0.1 * linear slop)
	static constexpr float overlapTolerance = 0.0005f;

	b2ShapeProxy GetShapeProxy(b2ShapeId shapeId)
	{
		switch (b2Shape_GetType(shapeId))
		{
		case b2_circleShape:
		{
			b2Circle circle = b2Shape_GetCircle(shapeId);
			return b2MakeProxy(&circle.center, 1, circle.radius);
		}
		case b2_capsuleShape:
		{
			b2Capsule capsule = b2Shape_GetCapsule(shapeId);
			return b2MakeProxy(&capsule.center1, 2, capsule.radius);
		}
		case b2_segmentShape:
		{
			b2Segment segment = b2Shape_GetSegment(shapeId);
			return b2MakeProxy(&segment.point1, 2, 0);
		}
		case b2_chainSegmentShape:
		{
			b2Segment segment = b2Shape_GetChainSegment(shapeId).segment;
			return b2MakeProxy(&segment.point1, 2, 0);
		}
		case b2_polygonShape:
		{
			b2Polygon polygon = b2Shape_GetPolygon(shapeId);
			return b2MakeProxy(polygon.vertices, polygon.count, polygon.radius);
		}
		default:
		{
			LOG_ERROR("GetShapeProxy: unsupported Box2D shape type.");
			b2Vec2 zero{ 0, 0 };
			return b2MakeProxy(&zero, 1, 0);
		}
		}
	}

	bool ShapeOverlapsPoint(b2ShapeId shapeId, Vec2f point)
	{
		//b2Shape_TestPoint compares the GJK distance to exactly zero, which
		//misses points sitting on edges (e.g. the internal seams of a
		//triangulated Polygon2D). Use the same tolerance as overlap queries.
		b2Vec2 center = point;
		b2DistanceInput input;
		input.proxyA = b2MakeProxy(&center, 1, 0);
		input.proxyB = GetShapeProxy(shapeId);
		input.transformA = b2Transform_identity;
		input.transformB = b2Body_GetTransform(b2Shape_GetBody(shapeId));
		input.useRadii = true;

		b2SimplexCache cache = { 0 };
		b2DistanceOutput output = b2ShapeDistance(&input, &cache, nullptr, 0);
		return output.distance < overlapTolerance;
	}

	bool Collider::ProxyOverlap(const b2ShapeProxy& proxy, b2Transform proxyTransform)
	{
		for (auto& shape : m_shapes)
		{
			b2DistanceInput input;
			input.proxyA = proxy;
			input.proxyB = GetShapeProxy(shape);
			input.transformA = proxyTransform;
			input.transformB = b2Body_GetTransform(b2Shape_GetBody(shape));
			input.useRadii = true;

			b2SimplexCache cache = { 0 };
			b2DistanceOutput output = b2ShapeDistance(&input, &cache, nullptr, 0);
			if (output.distance < overlapTolerance)
				return true;
		}
		return false;
	}

	bool Collider::ColliderOverlap(Collider* collider)
	{
		for (auto& shape : collider->m_shapes)
		{
			b2ShapeProxy proxy = GetShapeProxy(shape);
			b2Transform transform = b2Body_GetTransform(b2Shape_GetBody(shape));
			if (ProxyOverlap(proxy, transform))
				return true;
		}
		return false;
	}

	bool Collider::CircleOverlap(Vec2f point, float radius)
	{
		b2Vec2 center = point;
		b2ShapeProxy proxy = b2MakeProxy(&center, 1, radius);
		return ProxyOverlap(proxy, b2Transform_identity);
	}

	bool Collider::PointInside(Vec2f point)
	{
		for (auto& shape : m_shapes)
		{
			if (ShapeOverlapsPoint(shape, point))
				return true;
		}
		return false;
	}

	b2AABB Collider::GetAABB()
	{
		if (m_shapes.empty())
		{
			return b2AABB{ { 0, 0 }, { 0, 0 } };
		}
		b2AABB aabb = b2Shape_GetAABB(m_shapes[0]);
		for (size_t i = 1; i < m_shapes.size(); i++)
		{
			aabb = b2AABB_Union(aabb, b2Shape_GetAABB(m_shapes[i]));
		}
		return aabb;
	}

	void Collider::SetMaterial(float density, float friction, float restitution)
	{
		if (!m_shapes.empty())
		{
			LOG_WARN("Collider::SetMaterial called after the collider was added to a Body: ignored.");
			return;
		}
		m_density = density;
		m_friction = friction;
		m_restitution = restitution;
	}

	void Collider::SetBody(Body* body, const b2Filter& filter)
	{
		m_body = body;
		b2ShapeDef def = b2DefaultShapeDef();
		def.filter = filter;
		def.density = m_density;
		def.material.friction = m_friction;
		def.material.restitution = m_restitution;
		CreateShapes(def);
	}

	void Collider::SetFilter(const b2Filter& filter)
	{
		for (auto& shape : m_shapes)
		{
			b2Shape_SetFilter(shape, filter);
		}
	}

	void Collider::DestroyShapes()
	{
		for (auto& shape : m_shapes)
		{
			if (b2Shape_IsValid(shape))
			{
				b2DestroyShape(shape, false);
			}
		}
		m_shapes.clear();
		m_body = nullptr;
	}

	///
	///
	///Box
	///
	///

	Box2D::Box2D()
	{
		m_polygon = b2MakeBox(0.5f, 0.5f);
	}

	Box2D::Box2D(Vec2f dimensions)
	{
		m_polygon = b2MakeBox(dimensions.x * 0.5f, dimensions.y * 0.5f);
	}

	Box2D::Box2D(float width, float height)
	{
		m_polygon = b2MakeBox(width * 0.5f, height * 0.5f);
	}

	void Box2D::CreateShapes(const b2ShapeDef& def)
	{
		m_shapes.emplace_back(b2CreatePolygonShape(m_body->GetB2Body(), &def, &m_polygon));
	}

	void Box2D::SetupRender(ColliderRender* render)
	{
		render->wire = makesptr<WireRender>(m_polygon.count + 2);
		for (int i = 0; i < m_polygon.count; i++)
		{
			render->wire->AddPoint(m_polygon.vertices[i]);
		}
		render->wire->AddPoint(m_polygon.vertices[0]);
		render->wire->SetColor(Vec4f(0, 1, 0, 1));
	}

	///
	///
	///Circle
	///
	///

	Circle2D::Circle2D()
	{
		m_circle = b2Circle{ { 0, 0 }, 1.f };
	}
	Circle2D::Circle2D(float radius)
	{
		m_circle = b2Circle{ { 0, 0 }, radius };
	}

	void Circle2D::CreateShapes(const b2ShapeDef& def)
	{
		m_shapes.emplace_back(b2CreateCircleShape(m_body->GetB2Body(), &def, &m_circle));
	}

	void Circle2D::SetupRender(ColliderRender* render)
	{
		render->wire = makesptr<WireRender>(21);
		for (int i = 0; i < 21; i++)
		{
			auto dir = Math::AngleToVec((float)i / (float)20 * 360) * m_circle.radius;
			render->wire->AddPoint(dir);
		}
		render->wire->SetColor(Vec4f(0, 1, 0, 1));
	}

	///
	///
	///Polygon
	///
	///

	Polygon2D::Polygon2D()
	{
		m_points.emplace_back(std::vector<Vec2f>());
	}
	Polygon2D::Polygon2D(std::vector<Vec2f> points)
	{
		m_points.emplace_back(points);
	}

	void Polygon2D::CreateShapes(const b2ShapeDef& def)
	{
		std::vector<uint32_t> triangles = BakeTriangles();
		if (triangles.size() < 3)
		{
			LOG_ERROR("Polygon2D collider have less than 3 vertices. Cannot add collider to body.");
			return;
		}

		b2BodyId body = m_body->GetB2Body();
		for (size_t i = 0; i * 3 < triangles.size(); i++)
		{
			b2Vec2 vertices[3];
			for (size_t j = 0; j < 3; j++)
			{
				vertices[j] = m_points[0][triangles[i * 3 + j]];
			}
			b2Hull hull = b2ComputeHull(vertices, 3);
			if (hull.count < 3)
			{
				//Degenerate (collinear or duplicated points) triangle
				continue;
			}
			b2Polygon polygon = b2MakePolygon(&hull, 0);
			m_shapes.emplace_back(b2CreatePolygonShape(body, &def, &polygon));
		}
	}

	void Polygon2D::SetupRender(ColliderRender* render)
	{
		auto& points = m_points[0];
		render->wire = makesptr<WireRender>((unsigned int)points.size() + 2);
		for (auto& point : points)
		{
			render->wire->AddPoint(point);
		}
		if (!points.empty())
		{
			render->wire->AddPoint(points[0]);
		}
		render->wire->SetColor(Vec4f(0, 1, 0, 1));
	}

	void Polygon2D::AddPoint(Vec2f point)
	{
		m_points[0].emplace_back(point);
	}
	void Polygon2D::SetPoints(std::vector<Vec2f>& points)
	{
		m_points[0] = points;
	}
	std::vector<uint32_t> Polygon2D::BakeTriangles()
	{
		return mapbox::earcut(m_points);
	}
}
