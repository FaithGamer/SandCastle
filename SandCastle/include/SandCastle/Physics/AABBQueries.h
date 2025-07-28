#pragma once

#include <box2d/box2d.h>
#include <box2d/b2_distance.h>
#include "SandCastle/Core/Vec.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/Physics/Collider.h"
#include "SandCastle/Physics/Body.h"
#include "SandCastle/Physics/Bitmask.h"
namespace SandCastle
{

	/// @brief For internal use
	class QueryRaycastCallbackClosest : public b2RayCastCallback
	{
	public:
		QueryRaycastCallbackClosest(Vec2f start, Bitmask16 mask, RaycastResult* result);
		float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;

	public:
		RaycastResult* result;

	private:
		Vec2f m_start;
		Bitmask16 m_mask;
	};

	/// @brief For internal use
	class QueryRaycastCallbackAll : public b2RayCastCallback
	{
	public:
		QueryRaycastCallbackAll(Vec2f start, Bitmask16 mask, std::vector<RaycastResult>* Results);
		float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;

	public:
		std::vector<RaycastResult>* results;

	private:
		Vec2f m_start;
		Bitmask16 m_mask;
	};

	/// @brief For internal use
	class QueryB2ShapeOverlapAll : public b2QueryCallback
	{
	public:
		QueryB2ShapeOverlapAll(b2Shape* shape, Bitmask16 mask, std::vector<OverlapResult>* Results);
		bool ReportFixture(b2Fixture* fixture) override;

	public:
		std::vector<OverlapResult>* results;

	private:
		Bitmask16 m_mask;
		b2Shape* m_shape;
	};

	/// @brief For internal use
	class QueryBodyOverlapAll : public b2QueryCallback
	{
	public:
		QueryBodyOverlapAll(Body* body, Bitmask16 mask, std::vector<OverlapResult>* Results);
		bool ReportFixture(b2Fixture* fixture) override;

	public:
		std::vector<OverlapResult>* results;

	private:
		Bitmask16 m_mask;
		Body* m_body;
	};

	/// @brief For internal use
	class QueryBodyRaycastAll : public b2RayCastCallback
	{
	public:
		QueryBodyRaycastAll(Body* body, Bitmask16 mask, std::vector<OverlapResult>* Results);
		float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;

	public:
		std::vector<OverlapResult>* results;

	private:
		Bitmask16 m_mask;
		Body* m_body;
	};

	/// @brief For internal use
	class QueryPointInsideAll : public b2QueryCallback
	{
	public:
		QueryPointInsideAll(Vec2f point, Bitmask16 mask, std::vector<OverlapResult>* Results);
		bool ReportFixture(b2Fixture* fixture) override;

	public:
		std::vector<OverlapResult>* results;

	private:
		Vec2f m_point;
		Bitmask16 m_mask;
	};

}
