#pragma once

#include <SandCastle/SandCastle.h>

void Launch()
{
	//This must always be the first line of your main.
	SandCastle::Engine::Init();
	
	//Here you can do you own initialization (input maps, systems...)

	//Start the main loop of the engine.
	SandCastle::Engine::Launch();

	//This line won't be reached before the player close the game.
}
