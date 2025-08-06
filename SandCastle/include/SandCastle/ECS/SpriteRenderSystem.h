#pragma once

#include "SandCastle/ECS/System.h"
#include "SandCastle/Render/QuadRenderData.h"
#include "SandCastle/Render/SpriteRender.h"
#include "SandCastle/Render/Transform.h"

namespace SandCastle
{
	class SpriteRenderSystem : public System
	{
	public:
		SpriteRenderSystem();
		/// @brief Sort entities on the Y axis for a clean 2D rendering of superposed semi transparent fragment
		/// Disable only if you need the extra performance boost
		/// @param sort true by default
		void SetZSort(bool sort);
		bool GetZSort();
		void OnRender() override;
		void OnClearBatches();
		int GetUsedMethod() override;
		static QuadRenderData MakeQuadRenderDataFromSpriteRender(const SpriteRender* render, const Transform* transform);
	private:
		bool m_zSort;
	};
}

