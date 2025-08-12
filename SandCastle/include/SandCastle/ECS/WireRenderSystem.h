#pragma once

#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	class WireRenderSystem : public System
	{
	public:
		WireRenderSystem();
		void OnLateUpdate() override;
		int GetUsedMethod() override;
	};
}

#pragma once
