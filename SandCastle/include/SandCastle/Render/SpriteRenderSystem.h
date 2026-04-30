#pragma once

#include "SandCastle/ECS/System.h"
#include "SandCastle/Render/QuadRenderData.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	/// @brief Engine system that submits every (Transform, SpriteRender) pair to the renderer each frame.
	class SpriteRenderSystem : public System
	{
	public:
		SpriteRenderSystem();
		/// @brief Sort entities on the Y axis for a clean 2D rendering of superposed semi transparent fragment
		/// Disable only if you need the extra performance boost
		/// @param sort true by default
		void SetZSort(bool sort);
		bool GetZSort();
		void LateUpdate() override;
		/// @brief Internal: invoked when Renderer2D wipes its batches (e.g. on resize).
		void OnClearBatches();
		int GetUsedMethod() override;
		/// @brief Build the quad render payload for a single sprite/transform pair.
		static QuadRenderData MakeQuadRenderDataFromSpriteRender(const SpriteRender* render, const Transform* transform);
	private:
		bool m_zSort = false;
	};
}

