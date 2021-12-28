// Columns.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <SDL.h>
#include <SDL_ttf.h>
#include <unordered_map>

#include "DefaultGame.h"
#include "ColumnsExecutive.h"
#include "CommandLine.h"

#include "DTSimple.h"
#include "DTJsonSerializer.h"
#include "DTUtils.h"

using namespace std;

namespace
{
	const char* RecordArgumentName() { return "record"; };
	const char* PlaybackArgumentName() { return "playback"; }
}

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

bool InitializeGameComponents(const std::shared_ptr<geng::IGame>& pGame,
	const std::unordered_map<std::string,geng::cmdline::ArgValues>& cmdLineMap)
{
	// Recording/playback
	geng::columns::ExecutiveSettings execSettings;
	execSettings.pbMode = geng::PlaybackMode::None;

	if (cmdLineMap.count(RecordArgumentName()) > 0)
	{
		if (cmdLineMap.count(PlaybackArgumentName()) > 0)
		{
			pGame->LogError("Cannot specify both playback and record.");
			return false;
		}

		execSettings.pbMode = geng::PlaybackMode::Record;
		const auto& recordFileEntry = cmdLineMap.at(RecordArgumentName());
		execSettings.pbFileName = recordFileEntry.vals.at(0);
	}
	else if (cmdLineMap.count(PlaybackArgumentName()) > 0)
	{
		execSettings.pbMode = geng::PlaybackMode::Playback;
		const auto& playbackFileEntry = cmdLineMap.at(PlaybackArgumentName());
		execSettings.pbFileName = playbackFileEntry.vals.at(0);
	}

	// The executive initializes all other components
	auto pExecutive = std::make_shared<geng::columns::ColumnsExecutive>(execSettings);

	if (!pExecutive->AddToGame(pGame))
	{
		pGame->LogError("Unable to initialize the game -- see previous log for errors.");
		return false;
	}

	pGame->AddComponent(pExecutive);

	return true;
}


int main(int argc, char** argv)
{
	if (!InitSDL())
	{
		return -1;
	}

	geng::DefaultGameArgs gameArgs;
	gameArgs.msBreather = 1;
	gameArgs.msTimePerFrame = 20;
	auto pGame = geng::DefaultGame::CreateGame(gameArgs);

	std::vector<geng::cmdline::ArgDesc>
		cmdArgDescs{ geng::cmdline::ArgDesc(RecordArgumentName(), "r", true, 1,1),
				geng::cmdline::ArgDesc(PlaybackArgumentName(), "p", true, 1, 1) };
	std::unordered_map<std::string, geng::cmdline::ArgValues> argMap;
	std::string cmdLineError;
	if (!geng::cmdline::ProcessArgs(cmdArgDescs,
		argc,
		argv,
		argMap,
		cmdLineError))
	{
		pGame->LogError("Error on the command line:");
		pGame->LogError(cmdLineError.c_str());
		std::string cmdlUsage = geng::cmdline::GetUsageString(cmdArgDescs);
		pGame->LogError(cmdlUsage.c_str());
		return -1;
	}

	if (!InitializeGameComponents(pGame,argMap))
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

/*
void TestDT()
{
	using namespace geng::data;
	using Suite = simple::Suite;

	auto pMyDict = DTDict<Suite>(
		{
			{"name", DTElem<Suite>("BobSmith")},
			{"ismarried", DTElem<Suite>(false)},
			{"friends",
				DTList<Suite>(
					{
						DTElem<Suite>("John"),
						DTElem<Suite>("Paul"),
						DTElem<Suite>("Ringo"),
						DTElem<Suite>("George")
					}
				)
			},
			{"ages",
				DTList<Suite>(
					{
						DTElem<Suite>(10),
						DTElem<Suite>(16),
						DTElem<Suite>(45),
						DTElem<Suite>((double)2.5)
					}
				)
			}
		}
	);

	std::cerr << "Built\n";

	DTJsonTokenGenerator jsonTokens;
	SerializeDataTree(pMyDict, jsonTokens);

	std::string formattedJson
		= PrintJsonIndented(jsonTokens.GetTokenVector(), 3, 2, false);

	std::cerr << "Formatted:\n" << formattedJson << '\n';
}
*/