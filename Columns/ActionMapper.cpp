#include "ActionMapper.h"

#include <algorithm>

geng::ActionMapper::KeyInfo_* 
	geng::ActionMapper::GetInfoForKey(geng::KeyCode keyCode, bool incRef)
{
	auto itInfoObj = m_keyInfoMap.find(keyCode);

	KeyInfo_* pkinfo{ nullptr };
	if (itInfoObj != m_keyInfoMap.end())
	{
		pkinfo = &itInfoObj->second;
	}
	else
	{
		auto emplResult = m_keyInfoMap.emplace(keyCode, KeyInfo_(keyCode));
		pkinfo = &emplResult.first->second;
		m_keyStates.emplace_back(&pkinfo->state);
	}

	if (incRef)
	{
		++pkinfo->refCount;
	}

	return pkinfo;
}

void geng::ActionMapper::ReleaseKey(geng::KeyCode keyCode)
{
	auto itInfoObj = m_keyInfoMap.find(keyCode);

	if (itInfoObj != m_keyInfoMap.end())
	{
		KeyInfo_* pkinfo = &itInfoObj->second;

		--pkinfo->refCount;

		if (!pkinfo->refCount)
		{
			auto itState = std::find(m_keyStates.begin(), m_keyStates.end(), &pkinfo->state);
			
			// NOTE:  If the iterator above is end() then there is a bug in the code
			if (itState != m_keyStates.end())
			{
				m_keyStates.erase(itState);
			}

			m_keyInfoMap.erase(itInfoObj);
		}
	}
}

geng::ActionID geng::ActionMapper::CreateAction(const char* pName)
{
	std::string actionName(pName);

	auto itAction = m_actionNameMap.find(actionName);

	ActionID id{ 0 };
	if (itAction == m_actionNameMap.end())
	{
		id = m_actions.size();
		m_actions.emplace_back(id,actionName);
		m_actionNameMap.emplace(actionName, id);
	}
	else
	{
		id = itAction->second;
	}

	return id;
}

geng::ActionID geng::ActionMapper::GetAction(const char* pName) const
{
	std::string actionName(pName);

	auto itAction = m_actionNameMap.find(actionName);

	ActionID id{ 0 };
	if (itAction != m_actionNameMap.end())
	{
		return itAction->second;
	}

	return INVALID_ACTION;
}

bool geng::ActionMapper::OnFrame(geng::IInput* pInput, MouseState* pMouseState)
{
	// Get the states of all the keys in the state vector, then go through the entire action vector
	// and update the state of each action

	if (!pInput->QueryInput(pMouseState, m_keyStates.data(), m_keyStates.size()))
	{
		return false;
	}
	
	/*
			- When all keys are in "on" or "pressed" state, then the action is "on" or "starting"
			- Otherwise (at least one key is "off" or "released") the action is "off" or "ending"
			- The action is "on" if it was "on" or "starting" in the previous frame, otherwise "starting"
			- The action is "ending" if it was "on" or "starting" in the previous frame, otherwise "off"
	*/
	for (ActionMapping_& action : m_actions)
	{
		bool isOn{ true };

		for (KeyInfo_* actionKey : action.keyLocs)
		{
			if (actionKey->state.signal != KeySignal::KeyDown
				&& actionKey->state.signal != KeySignal::KeyPressed)
			{
				isOn = false;
				break;
			}
		}

		action.prevState = action.state;
		if (isOn)
		{
			if (action.state == ActionState::On || action.state == ActionState::Starting)
			{
				action.state = ActionState::On;
			}
			else
			{
				action.state = ActionState::Starting;
			}
		}
		else
		{
			if (action.state == ActionState::On || action.state == ActionState::Starting)
			{
				action.state = ActionState::Ending;
			}
			else
			{
				action.state = ActionState::Off;
			}
		}
	}

	return true;
}