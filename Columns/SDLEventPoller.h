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

			size_t idx = m_events.size() - 1;
			bool hasElements{ true };
			while (hasElements)
			{
				if (callback(const_cast<const SDL_Event&>(m_events[idx])))
				{
					m_events.erase(m_events.begin() + idx);
				}
				else
				{
					if (idx == 0)
					{
						hasElements = false;
					}
					else
					{
						--idx;
					}
				}
			}
		}

	private:
		std::vector<SDL_Event>   m_events;
	};


}