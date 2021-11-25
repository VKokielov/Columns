// Columns.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <SDL.h>
#include <SDL_ttf.h>

#include "DefaultGame.h"
#include "ColumnsExecutive.h"

using namespace std;

bool InitSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << "SDL SetUp: SDL_Init failed; SDL error " << SDL_GetError() << '\n';
		return false;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		std::cerr << "Unable to turn on LINEAR FILTERING\n";
	}

	if (TTF_Init() == -1)
	{
		std::cerr << "SDL SetUp: TTF_Init (font engine startup) failed; SDL-TTF error " << TTF_GetError()
		<< '\n';
		return false;
	}

	return true;
}

void DeinitSDL()
{
	SDL_Quit();
	TTF_Quit();
}

bool InitializeGameComponents(const std::shared_ptr<geng::IGame>& pGame)
{
	// The executive initializes all other components
	auto pExecutive = std::make_shared<geng::columns::ColumnsExecutive>();

	if (!pExecutive->AddToGame(pGame))
	{
		pGame->LogError("Unable to initialize the game -- see previous log for errors.");
		return false;
	}

	pGame->AddComponent(pExecutive);

	return true;
}

int main(int, char**)
{
	if (!InitSDL())
	{
		return -1;
	}

	geng::GameArgs gameArgs;
	gameArgs.msBreather = 1;
	gameArgs.msTimePerFrame = 20;
	auto pGame = geng::DefaultGame::CreateGame(gameArgs);
	
	if (!InitializeGameComponents(pGame))
	{
		std::cerr << "Could not initialize the game.\n";
		return -1;
	}

	// Run!
	if (!pGame->Run())
	{
		std::cerr << "Exited abnormally\n";
		pGame.reset();
		return -1;
	}

	std::cout << "Game finished -- exiting\n";

	// Destroy pGame to enable reasonably safe cleanup
	pGame.reset();

	DeinitSDL();

	return 0;
}
