#pragma once

#include "SandCastle/ECS/System.h"

namespace SandCastle
{
	class LineRendererSystem : public System
	{
	public:
		LineRendererSystem();
		void LateUpdate() override;
		void Render();
		int GetUsedMethod() override;
	};
}

