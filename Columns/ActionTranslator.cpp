#include "ActionTranslator.h"

geng::ActionTranslator::KeyIndex geng::ActionTranslator::GetOrCreateKeyIndex(KeyCode key)
{
	fprintf(stderr, "get-key %d\n", key);
	auto itKey = m_subKeyMap.find(key);
	if (itKey != m_subKeyMap.end())
	{
		return itKey->second;
	}

	// Create a new key. 

	KeyIndex keyIndex = m_keyStateVector.size();

	m_keyStateVector.emplace_back();
	m_keyStateVector.back().m_pkeyState->keyCode = key;
	m_keyStateRefs.emplace_back(m_keyStateVector.back().m_pkeyState.get());
	m_subKeyMap.emplace(key, keyIndex);
	
	if (m_pInput)
	{
		m_pInput->AddCode(key);
	}

	return keyIndex;
}

void geng::ActionTranslator::SetInput(const std::shared_ptr<IInput>& pInput)
{
	m_pInput = pInput;
}

void geng::ActionTranslator::OnMapping(ActionID actionId, const ActionMapping& mapping)
{
	auto itAction = m_actionMap.find(actionId);
	if (itAction != m_actionMap.end())   // Ignore actions that don't concern me
	{
		ActionInfo_& rInfo = itAction->second;

		rInfo.keyRefs.clear();
		for (const auto& keyGroup : mapping.keyGroups)
		{
			std::vector<KeyIndex>  transKeyGroup;
			for (KeyCode key : keyGroup)
			{
				KeyIndex kidx = GetOrCreateKeyIndex(key);
				transKeyGroup.emplace_back(kidx);
			}

			rInfo.keyRefs.emplace_back(std::move(transKeyGroup));
		}
	}
}

void geng::ActionTranslator::UpdateOnFrame(unsigned long frameId)
{
	if (!m_pInput)
	{
		return;
	}

	KeyboardState kbState;
	MouseState mouseState;

	m_pInput->QueryInput(&mouseState, &kbState, m_keyStateRefs.data(), m_keyStateRefs.size());

	unsigned int nOnKeys{ 0 };
	for (KeyInfo_& rKey : m_keyStateVector)
	{
		if (IsOnAction(rKey))
		{
			++nOnKeys;
		}
	}

	if (nOnKeys > 0)
	{
		fprintf(stderr, "on keys %d\n", nOnKeys);
	}

	// Go through all my actions and update their state
	// An action is ON when each key was switched to "on" at least once during the last
	//  frame, for at least one group

	for (auto& rActionState : m_actionMap)
	{
		bool actionOn{ false };
		for (const auto& keyGroup : rActionState.second.keyRefs)
		{
			if (rActionState.second.keyRefs.size() == nOnKeys)
			{
				bool keysOn{ true };
				for (KeyIndex index : keyGroup)
				{
					if (!IsOnAction(m_keyStateVector[index]))
					{
						keysOn = false;
						break;
					}
				}

				if (keysOn)  // Found a case where all the right keys are on
				{
					actionOn = true;
					break;
				}
			}
		}

		rActionState.second.actState = actionOn ? ActionState::On : ActionState::Off;
	}
}