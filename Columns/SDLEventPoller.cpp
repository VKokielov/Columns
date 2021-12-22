#include "SDLEventPoller.h"

geng::sdl::EventPoller::EventPoller()
	:BaseGameComponent("SDLEventPoller")
{

}

bool geng::sdl::EventPoller::Initialize(const std::shared_ptr<IGame>& pGame)
{
	m_pGame = pGame;
	return true;
}

void geng::sdl::EventPoller::OnFrame(const SimState& simState, const SimContextState* pCtxState)
{
	m_events.clear();

	SDL_Event evt;
	while (SDL_PollEvent(&evt))
	{
		if (evt.type == SDL_QUIT)
		{
			// Signal quit and ignore the rest of the queue
			m_pGame->Quit();
			break;
		}

		m_events.emplace_back(evt);
	}
}