#include "ActionMapper.h"

#include <algorithm>

geng::ActionMapper::ActionMapper(const char* pInputName)
	:BaseGameComponent("ActionMapper", GameComponentType::IO),
	m_inputName(pInputName)
{

}

bool geng::ActionMapper::Initialize(const std::shared_ptr<IGame>& pGame)
{
	GetComponentResult getResult;
	m_pInput = GetComponentAs<IInput>(pGame.get(), m_inputName.c_str(), getResult);

	if (!m_pInput)
	{
		pGame->LogError("ActionMapper: could not get input component");
		return false;
	}

	return true;
}

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
		id = (ActionID)m_actions.size();
		m_actions.emplace_back(id,actionName);
		m_actionNameMap.emplace(actionName, id);
	}
	else
	{
		id = (ActionID)itAction->second;
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
		return (ActionID)itAction->second;
	}

	return INVALID_ACTION;
}

void geng::ActionMapper::OnFrame(IFrameManager* pFrameManager)
{
	// Get the states of all the keys in the state vector, then go through the entire action vector
	// and update the state of each action

	
	if (!m_pInput->QueryInput(&m_mouseState, m_keyStates.data(), m_keyStates.size()))
	{
		return;
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
}