#include "pch.h"
#include "SandCastle/Physics/Collider.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Core/Math.h"

namespace SandCastle
{
	Box2D::Box2D()
	{
		m_shape.SetAsBox(1, 1);
	}

	Box2D::Box2D(Vec2f dimensions)
	{
		m_shape.SetAsBox(dimensions.x * 0.5f, dimensions.y * 0.5f);
	}

	Box2D::Box2D(float width, float height)
	{
		m_shape.SetAsBox(width * 0.5f, height * 0.5f);
	}

	void Box2D::SetBody(Body* body, b2Filter filter, UserData& userData)
	{
		//Create fixture and attach to body
		b2Body* b2B = body->GetB2Body();
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &m_shape;
		fixtureDef.userData.pointer = (uintptr_t)&userData;
		auto fixture = b2B->CreateFixture(&fixtureDef);

		fixture->SetFilterData(filter);
		m_body = body;

	}

	bool Box2D::B2ShapeOverlap(b2Shape* shape, b2Transform& transform)
	{
		return b2TestOverlap(&m_shape, 0, shape, 0, m_body->GetB2Body()->GetTransform(), transform);
	}

	bool Box2D::ColliderOverlap(Collider* collider)
	{
		return false;
	}

	bool Box2D::CircleOverlap(Vec2f point, float radius)
	{
		return false;
	}

	bool Box2D::PointInside(Vec2f point)
	{
		return false;
	}

	void Box2D::SetupRender(ColliderRender* render)
	{
		render->wire = makesptr<WireRender>(6);
		render->wire->AddPoint(m_shape.m_vertices[0]);
		render->wire->AddPoint(m_shape.m_vertices[1]);
		render->wire->AddPoint(m_shape.m_vertices[2]);
		render->wire->AddPoint(m_shape.m_vertices[3]);
		render->wire->AddPoint(m_shape.m_vertices[0]);
		render->wire->SetColor(Vec4f(0, 1, 0, 1));
	}

	b2AABB Box2D::GetAABB()
	{
		b2AABB aabb;
		m_shape.ComputeAABB(&aabb, m_body->GetB2Body()->GetTransform(), 0);
		return aabb;
	}

	///
	/// 
	///Circle
	///
	///

	Circle2D::Circle2D()
	{
		m_shape.m_radius = 1.f;
	}
	Circle2D::Circle2D(float radius)
	{
		m_shape.m_radius = radius;
	}

	void Circle2D::SetBody(Body* body, b2Filter filter, UserData& userData)
	{
		b2Body* b2B = body->GetB2Body();
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &m_shape;
		fixtureDef.userData.pointer = (uintptr_t)&userData;
		auto fixture = b2B->CreateFixture(&fixtureDef);
		fixture->SetFilterData(filter);
		m_body = body;
	}

	bool Circle2D::B2ShapeOverlap(b2Shape* shape, b2Transform& transform)
	{
		return b2TestOverlap(&m_shape, 0, shape, 0, m_body->GetB2Body()->GetTransform(), transform);
	}
	bool Circle2D::ColliderOverlap(Collider* collider)
	{
		return false;
	}
	bool Circle2D::CircleOverlap(Vec2f point, float radius)
	{
		return false;
	}
	bool Circle2D::PointInside(Vec2f point)
	{
		return false;
	}
	void Circle2D::SetupRender(ColliderRender* render)
	{
		render->wire = makesptr<WireRender>(21);
		for (int i = 0; i < 21; i++)
		{
			auto dir = Math::AngleToVec((float)i / (float)20 * 360)* m_shape.m_radius;
			render->wire->AddPoint(dir);
		}
		render->wire->SetColor(Vec4f(0, 1, 0, 1));
	}
	b2AABB Circle2D::GetAABB()
	{
		b2AABB aabb;
		m_shape.ComputeAABB(&aabb, m_body->GetB2Body()->GetTransform(), 0);
		return aabb;
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

	void Polygon2D::SetBody(Body* body, b2Filter filter, UserData& userData)
	{
		m_body = body;
		BakeTriangles();
		if (m_triangles.size() < 3)
		{
			LOG_ERROR("Polygon2D collider have less than 3 vertices. Cannot add collider to body.");
			return;
		}
		int i = 0;

		b2FixtureDef def;
		def.filter = filter;
		std::vector<Vec2f> debugTriangles; //to do to delete

		while (i * 3 < m_triangles.size())
		{
			b2Vec2* vertices = new b2Vec2[3];
			m_shapes.emplace_back(b2PolygonShape());
			for (int j = 0; j < 3; j++)
			{
				vertices[j] = m_points[0][m_triangles[i * 3 + j]];
				debugTriangles.emplace_back(m_points[0][m_triangles[i * 3 + j]]);
			}
			m_shapes.back().Set(vertices, 3);


			def.shape = &m_shapes.back();
			def.userData.pointer = (uintptr_t)&userData;

			b2Body* b2B = body->GetB2Body();
			auto fixture = b2B->CreateFixture(&def);

			delete[] vertices;
			i++;
		}
	}
	bool Polygon2D::B2ShapeOverlap(b2Shape* shape, b2Transform& transform)
	{
		for (int i = 0; i < m_shapes.size(); i++)
		{
			if (b2TestOverlap(&m_shapes[i], 0, shape, 0, m_body->GetB2Body()->GetTransform(), transform))
				return true;
		}
		return false;
	}
	bool Polygon2D::ColliderOverlap(Collider* collider)
	{
		return false;
	}
	bool Polygon2D::CircleOverlap(Vec2f point, float radius)
	{
		return false;
	}
	bool Polygon2D::PointInside(Vec2f point)
	{
		return false;
	}
	void Polygon2D::SetupRender(ColliderRender* render)
	{
		//to do
		//render->wire
	}
	void Polygon2D::AddPoint(Vec2f point)
	{
		m_points[0].emplace_back(point);
	}
	void Polygon2D::SetPoints(std::vector<Vec2f>& points)
	{
		m_points[0] = points;
	}
	void Polygon2D::BakeTriangles()
	{
		m_triangles = mapbox::earcut(m_points);
	}

	b2AABB Polygon2D::GetAABB()
	{
		b2AABB aabb;
		aabb.lowerBound.x = -99999999.f;
		aabb.lowerBound.y = -99999999.f;
		aabb.upperBound.x = 99999999.f;
		aabb.upperBound.y = 99999999.f;
		for (int i = 1; i < m_shapes.size(); i++)
		{
			b2AABB saabb;
			m_shapes[i].ComputeAABB(&saabb, m_body->GetB2Body()->GetTransform(), 0);
			if (saabb.lowerBound.x < aabb.lowerBound.x)
				aabb.lowerBound.x = saabb.lowerBound.x;
			if (saabb.lowerBound.y < aabb.lowerBound.y)
				aabb.lowerBound.y = saabb.lowerBound.y;
			if (saabb.upperBound.x > aabb.upperBound.x)
				aabb.upperBound.x = saabb.upperBound.x;
			if (saabb.upperBound.y > aabb.upperBound.y)
				aabb.upperBound.y = saabb.upperBound.y;
		}
		return aabb;
	}
}