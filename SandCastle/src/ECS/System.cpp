#include "pch.h"
#include "SandCastle/ECS/System.h"
#include "SandCastle/ECS/Systems.h"

namespace SandCastle
{
	std::vector<World*>& System::GetWorlds()
	{
		return Systems::GetWorlds();
	}
}