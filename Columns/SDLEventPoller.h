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

		IFrameListener* GetFrameListener() override;
		void OnFrame(IFrameManager* pManager) override;

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
	};


}