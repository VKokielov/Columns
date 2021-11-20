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
#include "SDLSetup.h"
#include "DefaultGame.h"

#include "ColumnsSDLRenderer.h"

using namespace std;

int main(int, char**)
{
	using CSim = geng::columns::ColumnsSim;

	geng::GameArgs gameArgs;
	gameArgs.frameArgs.msBreather = 1;
	gameArgs.frameArgs.msTimePerFrame = 20;

	auto pGame = geng::DefaultGame::CreateGame(gameArgs);

	// SETUP
	geng::sdl::SetupArgs setUpArgs{ SDL_INIT_VIDEO };
	auto pSetup = std::make_shared<geng::sdl::SetUp>(setUpArgs);
	pGame->AddComponent(pSetup);

	// EVENT POLLER
	auto pEventPoller = std::make_shared<geng::sdl::EventPoller>();
	pGame->AddComponent(pEventPoller);

	// INPUT
	auto pInput = std::make_shared<geng::sdl::Input>();
	pGame->AddComponent(pInput);

	// ACTION MAPPER
	auto pActionMapper = std::make_shared<geng::ActionMapper>("SDLInput");
	// Map keys to SDL keycodes
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

	pGame->AddComponent(pActionMapper);

	// SIMULATION
	geng::columns::ColumnsSimArgs simArgs;
	simArgs.actionThrottlePeriod = 200;
	simArgs.boardSize.x = 9;
	simArgs.boardSize.y = 24 + 3;  // 3 invisible squares on top
	simArgs.columnSize = 3;
	// NOTE:  This speed may change!
	simArgs.dropMilliseconds = 600;
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

	// Run!
	if (!pGame->Run())
	{
		std::cerr << "Exited abnormally\n";
		return -1;
	}

	std::cout << "Game finished -- exiting\n";
	return 0;
}
