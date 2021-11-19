#pragma once

#include <SDL.h>
#include <vector>

#include "BaseGameComponent.h"
#include "IFrameManager.h"

namespace geng::sdl
{
	// Gather together a stream of SDL events for the current frame

	class EventPoller : public BaseGameComponent, IFrameListener
	{
	public:
		EventPoller();

		void OnFrame(IFrameManager* pManager) override;

		template<typename F>
		void IterateEvents(F&& callback)
		{
			if (m_events.empty())
			{
				return;
			}

			for (auto itRev = m_events.rbegin();
				itRev != m_events.rend(); )
			{
				if (callback(const_cast<const SDL_Event&>(*itRev)))
				{
					itRev = m_events.erase(itRev);
				}
				else
				{
					++itRev;
				}
			}
		}

	private:
		std::vector<SDL_Event>   m_events;
	};


}