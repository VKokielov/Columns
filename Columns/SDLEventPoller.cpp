#include "SDLEventPoller.h"

geng::sdl::EventPoller::EventPoller()
	:BaseGameComponent("SDLEventPoller", GameComponentType::Simulation)
{

}

void geng::sdl::EventPoller::OnFrame(IFrameManager* pManager)
{
	m_events.clear();

	SDL_Event evt;
	while (SDL_PollEvent(&evt))
	{
		if (evt.type == SDL_QUIT)
		{
			SimState simstate;
			simstate.quality = SimQuality::Stopped;
			pManager->UpdateSimState(simstate, FID_QUALITY);
			break;
		}

		m_events.emplace_back(evt);
	}
}