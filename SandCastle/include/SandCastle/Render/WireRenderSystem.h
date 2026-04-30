#pragma once

#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	/// @brief Engine system that submits every WireRender + Transform pair to the renderer each frame.
	class WireRenderSystem : public System
	{
	public:
		WireRenderSystem();
		void LateUpdate() override;
		int GetUsedMethod() override;
	};
}

#pragma once
