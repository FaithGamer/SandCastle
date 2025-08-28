#pragma once

#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	class WireRenderSystem : public System
	{
	public:
		WireRenderSystem();
		void LateUpdate() override;
		int GetUsedMethod() override;
	};
}

#pragma once
