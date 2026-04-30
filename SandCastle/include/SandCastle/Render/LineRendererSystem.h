#pragma once

#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	/// @brief Engine system that submits every LineRenderer + Transform pair to the renderer each frame.
	class LineRendererSystem : public System
	{
	public:
		LineRendererSystem();
		void LateUpdate() override;
		/// @brief Forward all live LineRenderers to Renderer2D::DrawLine.
		void Render();
		int GetUsedMethod() override;
	};
}

