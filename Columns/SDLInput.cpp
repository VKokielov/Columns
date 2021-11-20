#include "SDLInput.h"

#include <ctime>

geng::sdl::Input::Input()
	:TemplatedGameComponent<IInput>("SDLInput", GameComponentType::Simulation)
{
	std::random_device device;
	m_generator.seed(device());
}

geng::IFrameListener* geng::sdl::Input::GetFrameListener()
{
	return this;
}

bool geng::sdl::Input::Initialize(const std::shared_ptr<IGame>& pGame)
{
	GetComponentResult getResult;
	m_pEventPoller = GetComponentAs<EventPoller>(pGame.get(), "SDLEventPoller", getResult);

	if (getResult == GetComponentResult::NoComponent)
	{
		pGame->LogError("SDLInput: cannot initialize -- missing SDLEventPoller");
		return false;
	}
	else if (getResult == GetComponentResult::ComponentWrongType)
	{
		pGame->LogError("SDLInput: cannot initialize -- SDLEventPoller not of the right type");
		return false;
	}

	return true;
}

void geng::sdl::Input::AddCode(KeyCode code)
{
	if (m_state.count(code) == 0)
	{
		m_state.emplace(code, KeyData_());
	}
}

unsigned long geng::sdl::Input::GetRandomNumber(unsigned long lowerBound, unsigned long upperBound)
{
	return m_generator() % (upperBound + 1 - lowerBound) + lowerBound;
}

bool geng::sdl::Input::QueryInput(MouseState* pMouseState,
	geng::KeyState** ppKeyStates,
	size_t nKeyStates)
{
	for (size_t i = 0; i < nKeyStates; ++i)
	{
		auto itCode = m_state.find(ppKeyStates[i]->keyCode);
		if (itCode == m_state.end())
		{
			return false;
		}

		ppKeyStates[i]->signal = itCode->second.signal;
	}

	return true;
}

void geng::sdl::Input::OnFrame(IFrameManager* pManager)
{
	++m_frame;

	// Update "pressed" keys to "down" and "released" keys to "up"
	std::unordered_set<KeyCode>  nextUpdated;

	for (KeyCode kc : m_updatedKeys)
	{
		auto itKey = m_state.find(kc);

		if (itKey->second.signal == KeySignal::KeyPressed)
		{
			if (itKey->second.shortPress && itKey->second.updateFrame == m_frame - 1)
			{
				itKey->second.signal = KeySignal::KeyReleased;
				nextUpdated.emplace(kc);
			}
			itKey->second.signal = KeySignal::KeyDown;
//			fprintf(stderr, "re-updating %d to %d\n", itKey->first, itKey->second);
		}
		else if (itKey->second.signal == KeySignal::KeyReleased)
		{
			itKey->second.signal = KeySignal::KeyUp;
//			fprintf(stderr, "re-updating %d to %d\n", itKey->first, itKey->second);
		}
	}
	m_updatedKeys = std::move(nextUpdated);

	auto evtHandler = [this](const SDL_Event& rEvent)
	{
		if (rEvent.type != SDL_KEYDOWN && rEvent.type != SDL_KEYUP)
		{
			return false;
		}

//		fprintf(stderr, "Key event received\n");
		KeyCode kcod = rEvent.key.keysym.sym;

		auto itKey = m_state.find(kcod);

		if (itKey == m_state.end())
		{
			return false;
		}

//		fprintf(stderr, "Key event found\n");

		// Set the key state
		itKey->second.shortPress
			= rEvent.type == SDL_KEYUP
			  && itKey->second.updateFrame == m_frame
			  && itKey->second.signal == KeySignal::KeyPressed;

		itKey->second.updateFrame = m_frame;

		itKey->second.signal = rEvent.type == SDL_KEYDOWN ? KeySignal::KeyPressed
			: KeySignal::KeyReleased;

//		fprintf(stderr, "Updating %d to %d\n", itKey->first, itKey->second);

		m_updatedKeys.emplace(kcod);
		return true;
	};

	m_pEventPoller->IterateEvents(evtHandler);
}