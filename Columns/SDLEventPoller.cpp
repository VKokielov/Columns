#include "SDLEventPoller.h"

geng::sdl::EventPoller::EventPoller()
	:BaseGameComponent("SDLEventPoller", GameComponentType::Simulation)
{

}

geng::IFrameListener* geng::sdl::EventPoller::GetFrameListener()
{
	return this;
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

		/*
		if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP)
		{
			fprintf(stderr, "Event %d for key %d\n", evt.type, (int)evt.key.keysym.sym);
		}
		*/

		m_events.emplace_back(evt);
	}
}