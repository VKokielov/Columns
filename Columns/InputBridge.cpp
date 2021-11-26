#include "InputBridge.h"
#include "KeyDebug.h"

geng::InputBridge::InputBridge(const char* pName, const std::shared_ptr<IInput>& pUnderlying)
	:TemplatedGameComponent<IInput>(pName),
	m_pUnderlying(pUnderlying)
{

}

// Yes, this was copied from SDLInput
// TODO
void geng::InputBridge::AddCode(KeyCode code)
{
	if (m_keyState.count(code) == 0)
	{
		KeyData_ kdata;
		kdata.state.keyCode = code;
		kdata.state.finalState = KeySignal::KeyUp;
		kdata.state.numChanges = 0;

		auto emplaceResult = m_keyState.emplace(code, kdata);
		m_stateRefs.emplace_back(&(emplaceResult.first->second.state));

		m_pUnderlying->AddCode(code);
	}
}

bool geng::InputBridge::ForceState(const KeyState& keyState)
{
	auto itState = m_keyState.find(keyState.keyCode);
	if (itState != m_keyState.end())
	{
		itState->second.state = keyState;
		return true;
	}

	return false;
}

bool geng::InputBridge::QueryInput(MouseState* pMouseState,
	KeyboardState* pkeyboardState,
	KeyState** ppKeyStates,
	size_t nKeyStates)
{
	for (size_t i = 0; i < nKeyStates; ++i)
	{
		auto itCode = m_keyState.find(ppKeyStates[i]->keyCode);
		if (itCode == m_keyState.end())
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

void geng::InputBridge::OnFrame(const SimState& simState, const SimContextState* pContextState)
{
	if (pContextState->focus.curValue)
	{
		// In focus.  Get all the elements from the underlying input and count how many of them
		// are actually down
		/*
#ifndef NDEBUG
		if (g_keyPressedThisFrame)
		{
			fprintf(stderr, "input bridge Key pressed!\n");
		}
#endif
		*/

		m_pUnderlying->QueryInput(nullptr, nullptr, m_stateRefs.data(), m_stateRefs.size());

		m_downKeys = 0;
		for (KeyState* pKey : m_stateRefs)
		{
			if (pKey->finalState == KeySignal::KeyDown)
			{
				++m_downKeys;
			}
		}
	}
	else if (pContextState->focus.prevValue)
	{
		// Set all keys to "Up"
		m_downKeys = 0;
		for (KeyState* pKey : m_stateRefs)
		{
			pKey->finalState = KeySignal::KeyUp;
		}
	}
}