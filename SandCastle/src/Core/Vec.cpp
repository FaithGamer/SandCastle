#include "pch.h"
#include "SandCastle/Core/Vec.h"

/*#include "SandCastle/Engine.h"

#include "SandCastle/Core/Log.h"
using namespace SandCastle;

void SandCastle::VectorUnitTest() 
{

	SandCastle::Engine::Init();

	LOG_INFO("Vector3");
	LOG_INFO("--------");
	LOG_INFO("Creating A = Vec3<float>(2, 3, 4).");
	LOG_INFO("Creating B = Vec3<float>(-1, 6, -8).");
	auto A = Vec3f(2, 3, 4);
	auto B = Vec3f(-1, 6, -8);
	LOG_INFO("A ToString: " + A.ToString());
	auto ANorm = A.Normalized();
	LOG_INFO("A normalized: " + ANorm.ToString());
	auto AClamped = A.ClampedMagnitude(2);
	LOG_INFO("A clamped with magnitude of 2: " + AClamped.ToString());
	auto ApB = A + B;
	LOG_INFO("A + B: " + ApB.ToString());
	auto AMovedTowardB = A.MovedTowards(B, 1);
	LOG_INFO("A moved toward B with magnitude of 1: " + AMovedTowardB.ToString());
	auto ACopy = A;
	ACopy += B;
	LOG_INFO("A += B: " + ACopy.ToString());
	LOG_INFO("Angle of -1, 0, 0 from 0, 1, 0: " + std::to_string(Vec3<float>(-1, 0, 0).Angle(Vec3f(0, 1, 0))));
	auto Am = -A;
	LOG_INFO("A negative: " + Am.ToString());
	LOG_INFO("A dot B: " + std::to_string(A.Dot(B)));
	LOG_INFO("A cross B: " + A.Cross(B).ToString());
	LOG_INFO("A Reflected on plane B: " + A.Reflected(B).ToString());
}*/