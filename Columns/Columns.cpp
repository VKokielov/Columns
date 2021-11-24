// Columns.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <utility>
#include <algorithm>
#include <string>
#include <SDL.h>

#include "SimStateDispatcher.h"

#include "SDLInput.h"
#include "SDLEventPoller.h"
#include "ColumnsSim.h"
#include "ActionMapper.h"
#include "DefaultGame.h"
#include "ResourceLoader.h"

#include "RawMemoryResource.h"
#include "TrueTypeFont.h"

#include "ColumnsSDLRenderer.h"

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

void InitializeGameComponents(geng::IGame* pGame)
{
	// It's important to keep these in a separate function so that the shared_ptrs
	// are released before the game loop starts.  Unfortunately some libraries 
	// don't like to have functions called on "final destruction".
	using CSim = geng::columns::ColumnsSim;

//	InitializeResourceLoader(pGame);

	// EVENT POLLER

	// INPUT

	// Map keys to SDL keycodes
	/*
	auto dropAction = pActionMapper->CreateAction(CSim::GetDropActionName());
	pActionMapper->MapAction(dropAction, SDLK_s);

	auto leftAction = pActionMapper->CreateAction(CSim::GetShiftLeftActionName());
	pActionMapper->MapAction(leftAction, SDLK_a);

	auto rightAction = pActionMapper->CreateAction(CSim::GetShiftRightActionName());
	pActionMapper->MapAction(rightAction, SDLK_d);

	auto rotateAction = pActionMapper->CreateAction(CSim::GetRotateActionName());
	pActionMapper->MapAction(rotateAction, SDLK_SPACE);

	auto permuteAction = pActionMapper->CreateAction(CSim::GetPermuteActionName());
	pActionMapper->MapAction(permuteAction, SDLK_r);
	*/

	// SIMULATION
	geng::columns::ColumnsSimArgs simArgs;
	simArgs.actionThrottlePeriod = 250;
	simArgs.dropThrottlePeriod = 150;
	simArgs.boardSize.x = 9;
	simArgs.boardSize.y = 24 + 3;  // 3 invisible squares on top
	simArgs.columnSize = 3;
	// NOTE:  This speed may change!
	simArgs.dropMilliseconds = 600;
	simArgs.flashMilliseconds = 300;
	simArgs.flashCount = 3;
	//simArgs.dropMilliseconds = 400;
	simArgs.pInputName = "SDLInput";

	auto pSim = std::make_shared<geng::columns::ColumnsSim>(simArgs);
	pGame->AddComponent(pSim);

	// RENDERER
	geng::columns::ColumnsRenderArgs renderArgs;
	renderArgs.windowX = 640 + 320;
	renderArgs.windowY = 480 + 240;
	renderArgs.renderShadow = 4;
	auto pRenderer = std::make_shared<geng::columns::ColumnsSDLRenderer>(renderArgs);
	pGame->AddComponent(pRenderer);
}

int main(int, char**)
{
	if (!InitSDL())
	{
		return -1;
	}

	geng::GameArgs gameArgs;
	gameArgs.frameArgs.msBreather = 1;
	gameArgs.frameArgs.msTimePerFrame = 20;
	auto pGame = geng::DefaultGame::CreateGame(gameArgs);
	InitializeGameComponents(pGame.get());

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
