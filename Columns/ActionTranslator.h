#pragma once

#include "ActionMapper.h"
#include "IInput.h"

namespace geng
{
	enum class ActionState
	{
		Off,
		On,
		Invalid
	};

	// Class that translates input values to actions based on the key states
	// in an input component
	
	// TODO:  Support action recording and playback

	class ActionTranslator : public IActionMappingListener
	{
	private:
		using KeyIndex = size_t;
		struct ActionInfo_
		{
			std::vector<std::vector<KeyIndex> > keyRefs;
			ActionState actState;
		};

		struct KeyInfo_
		{
			// The extra indirection is ugly but needed to keep the 
			// structure stable when the hosting vector grows
			std::unique_ptr<KeyState>   m_pkeyState;

			KeyInfo_()
				:m_pkeyState(new KeyState())
			{

			}
		};

	public:
		
		template<typename I>
		bool InitActions(ActionMapper* pMapper, I bActionNames, I eActionNames)
		{
			m_actionMap.clear();

			// Look up the action names given in the mapper and store them as entries
			while (bActionNames != eActionNames)
			{
				ActionID actId = pMapper->GetAction(*bActionNames);
				if (actId == INVALID_ACTION)
				{
					return false;
				}

				m_actionMap.emplace(actId, ActionInfo_());
				++bActionNames;
			}
			return true;
		}

		void SetInput(const std::shared_ptr<IInput>& pInput);
		void OnMapping(ActionID actionId, const ActionMapping& mapping) override;
		void UpdateOnFrame(unsigned long frameId);

		ActionState GetActionState(ActionID actionId) const
		{
			auto itAction = m_actionMap.find(actionId);
			if (itAction != m_actionMap.end())
			{
				return itAction->second.actState;
			}
			return ActionState::Invalid;
		}

	private:
		bool IsOnAction(const KeyInfo_& key)
		{
			// 1. The key is down at the end of the frame
			// 2. The key is up and number of changes is not 0 (still up) nor 1 (down, up).
			return key.m_pkeyState->finalState == KeySignal::KeyDown
				|| key.m_pkeyState->numChanges > 1;
		}

		// Get a key, ensuring that it's added to the input object if one exists
		KeyIndex GetOrCreateKeyIndex(KeyCode key);
	//	KeyInfo_& GetKey(KeyIndex index);

		std::shared_ptr<IInput> m_pInput;

		std::unordered_map<ActionID, ActionInfo_>  m_actionMap;
		// Keys to which we subscribe, along with an index into the key state vector
		// By using an index we avoid continuous and pointless map lookups
		std::unordered_map<KeyCode, KeyIndex> m_subKeyMap;
		std::vector<KeyInfo_> m_keyStateVector;

		std::vector<KeyState*>  m_keyStateRefs;
	};
}