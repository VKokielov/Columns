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

using namespace std;

int main(int, char**)
{
	using CSim = geng::columns::ColumnsSim;

	// SETUP
	auto pSetup = std::make_shared<geng::sdl::SetUp>(640, 480);

	// EVENT POLLER
	auto pEventPoller = std::make_shared<geng::sdl::EventPoller>();

	// INPUT
	auto pInput = std::make_shared<geng::sdl::Input>();

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

	// SIMULATION
	geng::columns::ColumnsSimArgs simArgs;
	simArgs.actionThrottlePeriod = 100;
	simArgs.boardSize.x = 9;
	simArgs.boardSize.y = 24 + 3;  // 3 invisible squares on top
	simArgs.columnSize = 3;
	// NOTE:  This speed may change!
	simArgs.dropMilliseconds = 400;
	simArgs.pInputName = "SDLInput";

	auto pSim = std::make_shared<geng::columns::ColumnsSim>(simArgs);
	
	// RENDERER

	// Game!
	geng::GameArgs gameArgs;
	gameArgs.timePerFrame = 20;

	geng::DefaultGame game{ gameArgs };

	// Run!
	if (!game.Run())
	{
		std::cerr << "Exited abnormally\n";
		return -1;
	}

	std::cout << "Game finished -- exiting\n";
	return 0;
}
