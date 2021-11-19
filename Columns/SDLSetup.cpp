#include "SDLSetup.h"

#include <SDL.h>
#include <sstream>

bool geng::sdl::SetUp::Initialize(const std::shared_ptr<IGame>& pGame)
{
	if (SDL_Init(m_args.initFlags) < 0)
	{
		std::stringstream ssm;
		ssm << "SDL SetUp: SDL_Init failed; SDL error " << SDL_GetError();
		std::string sErr = ssm.str();
		pGame->LogError(sErr.c_str());
		return false;
	}

	return true;
}

void geng::sdl::SetUp::WindDown(const std::shared_ptr<IGame>& pGame)
{
	SDL_Quit();
}