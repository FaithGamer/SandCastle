#pragma once

#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Core/Bitmask.h"
#include "SandCastle/Physics/Body.h"

namespace SandCastle
{
	/// @brief Everything for collision and physics.
	/// Currently doesn't take into consideration different worlds.
	class Physics : public Singleton<Physics>
	{
	public:
		Physics();
		~Physics();

		static void RaycastClosest(RaycastResult& result, Vec2f start, Vec2f end, Bitmask16 mask = 65535);
		static void RaycastAll(std::vector<RaycastResult>& results, Vec2f start, Vec2f end, Bitmask16 mask = 65535);
		static void CircleOverlap(std::vector<OverlapResult>& results, Vec2f pos, float radius, Bitmask16 mask = 65535);
		static void PointInside(std::vector<OverlapResult>& results, Vec2f pos, Bitmask16 mask = 65535);
		static void BodyOverlap(std::vector<OverlapResult>& results, Body* body, Bitmask16 mask = 65535);
		static int GetBodyCount();

		/// @brief Add a collision layer with a custom name
		/// Generally call this method a bunch of time before launching the engine and creating Bodies
		/// @param layerName
		static void AddLayer(String layerName)
		{
			Instance()->m_layers.AddFlag(layerName);
		}
		/// @brief Create a bitmask for the given collision layers.
		/// Layers must have been added first using AddLayer.
		/// @return Bitmask16 of layers
		template <typename ...Str>
		static Bitmask16 GetLayerMask(Str... layers)
		{
			return Instance()->m_layers.GetMask(layers...);
		}

		/// @brief Enable/Disable drawing the colliders wireframe
		/// @param draw
		static void DrawColliders(bool draw);
		/// @brief Underlying Box2D world id (advanced usage: joints, stepping, etc.).
		static b2WorldId GetB2World();
	private:

		b2WorldId m_world;
		friend Singleton<Physics>;
		Filter16 m_layers;
	};
}
