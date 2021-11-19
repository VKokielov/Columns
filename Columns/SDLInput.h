#pragma once

#include "IInput.h"
#include "IFrameManager.h"
#include "BaseGameComponent.h"
#include "SDLEventPoller.h"

#include <unordered_map>
#include <unordered_set>
#include <random>

namespace geng::sdl
{

	class Input : public BaseGameComponent,
		public IInput,
		public IFrameListener
	{
	public:
		Input();
		bool Initialize(const std::shared_ptr<IGame>& pGame) override;

		void AddCode(KeyCode code) override;
		void OnFrame(IFrameManager* pManager) override;
		unsigned long GetRandomNumber(unsigned long min, unsigned long upperBound) override;
		bool QueryInput(MouseState* pMouseState,
			KeyState** ppKeyStates,
			size_t nKeyStates) override;

	private:
		// The event poller polls events for the frame
		std::shared_ptr<EventPoller>   m_pEventPoller;
		std::unordered_map<KeyCode, KeySignal>   m_state;
		std::unordered_set<KeyCode>  m_updatedKeys;

		// Random numbers
		std::mt19937_64  m_generator;
	};


}