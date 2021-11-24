#pragma once

#include <SDL.h>
#include <vector>

#include "BaseGameComponent.h"

namespace geng::sdl
{
	// Gather together a stream of SDL events for the current frame

	class EventPoller : public BaseGameComponent, 
		public IGameListener,
		public std::enable_shared_from_this<EventPoller>
	{
	public:
		EventPoller();

		void OnFrame(const SimState& simState, const SimContextState* pContextState) override;

		template<typename F>
		void IterateEvents(F&& callback)
		{
			if (m_events.empty())
			{
				return;
			}

			for (const SDL_Event& evt : m_events)
			{
				callback(evt);
			}
		}

	private:
		std::vector<SDL_Event>   m_events;
		std::shared_ptr<IGame>   m_pGame;
	};


}