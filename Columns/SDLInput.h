#pragma once

#include "IInput.h"
#include "BaseGameComponent.h"
#include "SDLEventPoller.h"
#include "IGame.h"

#include <unordered_map>
#include <unordered_set>

namespace geng::sdl
{

	class Input : public TemplatedGameComponent<IInput>,
		public IGameListener
	{
	private:
		struct KeyData_
		{
			KeyState state;
		};
	public:

		Input();
		bool Initialize(const std::shared_ptr<IGame>& pGame) override;
		bool ForceState(const KeyState& keyState) override;

		void AddCode(KeyCode code) override;
		void OnFrame(const SimState& simState, const SimContextState* pContextState) override;
		bool QueryInput(MouseState* pMouseState,
			KeyboardState* pkeyboardState,
			KeyState** ppKeyStates,
			size_t nKeyStates) override;

	private:
		// The event poller polls events for the frame
		std::shared_ptr<EventPoller>   m_pEventPoller;
		std::unordered_map<KeyCode, KeyData_>   m_state;
		unsigned int m_downKeys{ 0 };

	};


}