#include "SDLInput.h"
#include "KeyDebug.h"

#include <ctime>

geng::sdl::Input::Input()
	:TemplatedGameComponent<IInput>("SDLInput")
{

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
		KeyData_ kdata;
		kdata.state.keyCode = code;
		kdata.state.finalState = KeySignal::KeyUp;
		kdata.state.numChanges = 0;

		m_state.emplace(code, kdata);
	}
}


bool geng::sdl::Input::QueryInput(MouseState* pMouseState,
	KeyboardState* pkeyboardState,
	KeyState** ppKeyStates,
	size_t nKeyStates)
{
	for (size_t i = 0; i < nKeyStates; ++i)
	{
		auto itCode = m_state.find(ppKeyStates[i]->keyCode);
		if (itCode == m_state.end())
		{
			return false;
		}

		*ppKeyStates[i] = itCode->second.state;
	}

	if (pkeyboardState)
	{
		pkeyboardState->numKeysDownInFrame = m_downKeys;
	}

	return true;
}

bool geng::sdl::Input::ForceState(const KeyState& keyState)
{
	auto itState = m_state.find(keyState.keyCode);
	if (itState != m_state.end())
	{
		itState->second.state = keyState;
		return true;
	}

	return false;
}

void geng::sdl::Input::OnFrame(const SimState& simState, const SimContextState* pContextState)
{
	// Reset the keystate
	for (auto& rKeyPair : m_state)
	{
		rKeyPair.second.state.numChanges = 0;
	}

	m_downKeys = 0;

#ifndef NDEBUG
	g_keyChangedThisFrame = false;
#endif

	auto evtHandler = [this](const SDL_Event& rEvent)
	{
		if (rEvent.type != SDL_KEYDOWN && rEvent.type != SDL_KEYUP)
		{
			return false;
		}

		KeyCode kcod = rEvent.key.keysym.sym;

		auto itKey = m_state.find(kcod);

		if (itKey == m_state.end())
		{
			return false;
		}

#ifndef NDEBUG
		g_keyChangedThisFrame = true;
#endif

		KeySignal newSignal = rEvent.type == SDL_KEYDOWN ? KeySignal::KeyDown : KeySignal::KeyUp;

		if (newSignal != itKey->second.state.finalState)
		{
			++itKey->second.state.numChanges;
			itKey->second.state.finalState = newSignal;
			
			rEvent.type == SDL_KEYDOWN ? ++m_downKeys : --m_downKeys;
		}

		return true;
	};

	m_pEventPoller->IterateEvents(evtHandler);
	
	/*
#ifndef NDEBUG
	if (g_keyChangedThisFrame)
	{
		fprintf(stderr, ">>>>\n");

		for (auto& rKeyPair : m_state)
		{
			fprintf(stderr, "key %d state %d numchanges %d\n",
				rKeyPair.first, rKeyPair.second.state.finalState,
				rKeyPair.second.state.numChanges);
		}
		fprintf(stderr, "<<<<\n");
	}
#endif
	*/
	
}