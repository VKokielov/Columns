#include "SDLSetup.h"

#include <SDL.h>
#include <sstream>
#include <SDL_ttf.h>

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

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		pGame->LogError("Unable to turn on LINEAR FILTERING");
	}

	if (TTF_Init() == -1)
	{
		std::stringstream ssm;
		ssm << "SDL SetUp: TTF_Init (font engine startup) failed; SDL-TTF error " << TTF_GetError();
		std::string sErr = ssm.str();
		pGame->LogError(sErr.c_str());
		return false;
	}

	return true;
}

void geng::sdl::SetUp::WindDown(const std::shared_ptr<IGame>& pGame)
{

}