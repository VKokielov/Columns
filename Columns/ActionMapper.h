#pragma once

#include <unordered_map>
#include <vector>
#include <cinttypes>
#include <optional>
#include <string>
#include <type_traits>
#include <iterator>

#include "IInput.h"

namespace geng
{
	using ActionID = long;

	constexpr ActionID INVALID_ACTION = -1L;

	// This utility class maps "virtual actions" to key codes.
	// Two or more keys can be mapped to a single action, in which case the "on" state is
	// defined by all keys being pressed.
	/*
		More precisely:
			- When all keys are in "on" or "pressed" state, then the action is "on" or "starting"
			- Otherwise (at least one key is "off" or "released") the action is "off" or "ending"
			- The action is "on" if it was "on" or "starting" in the previous frame, otherwise "starting"
			- The action is "ending" if it was "on" in the previous frame, otherwise "off"
	*/

	enum class ActionState
	{
		Initial,
		Off,
		Starting,
		On,
		Ending
	};

	class ActionMapper
	{
	private:
		struct KeyInfo_
		{
			KeyState state{};
			unsigned int refCount{ 0 };

			KeyInfo_(KeyCode code)
			{
				state.keyCode = code;
			}
		};

		struct ActionMapping_
		{
			ActionID id{ INVALID_ACTION };
			std::string actionName;
			ActionState prevState{ ActionState::Initial };
			ActionState state{ ActionState::Initial };
			std::vector<KeyInfo_*> keyLocs;

			ActionMapping_(ActionID id_, const std::string& actionName_)
				:id(id_),
				actionName(actionName_)
			{ }
		};

	public:
		// Action management 

		// This function will return an existing action ID if the action already exists
		ActionID CreateAction(const char* pName);
		// Get an existing action
		ActionID GetAction(const char* pName) const;

		template<typename Iter>
		bool MapAction(ActionID actionId, Iter bCodes, Iter eCodes)
		{
			static_assert(std::is_same_v<decltype(*bCodes), KeyCode>,
				"bCodes: expecting KeyCode container's iterator");
			static_assert(std::is_same_v<decltype(*eCodes), KeyCode>,
				"eCodes: expecting KeyCode container's iterator");

			// Clear the previous mapping of the given action and add the keycodes
			// in the array as mappings
			if (actionId < 0 || actionId >= m_actions.size())
			{
				return false;
			}

			ActionMapping_& actMapping = m_actions[actionId];

			// Clear the previous mapping
			for (KeyInfo_* pInfo : actMapping.keyLocs)
			{
				ReleaseKey(pInfo->state.keyCode);
			}

			actMapping.keyLocs.clear();

			// Create a new mapping
			while (bCodes != eCodes)
			{
				KeyCode keyCode = *bCodes;
				
				KeyInfo_* pInfo = GetInfoForKey(keyCode, true);
				actMapping.keyLocs.emplace_back(pInfo);
				++bCodes;
			}

			return true;
		}

		// Action states
		// The mouse state paramter is simply passed through
		bool OnFrame(IInput* pInput, MouseState* pMouseState = nullptr);

		ActionState GetActionState(ActionID actionId) const
		{
			if (actionId < 0 || actionId >= m_actions.size())
			{
				return ActionState::Initial;
			}

			return m_actions[actionId].state;
		}
		
	private:

		// KEY MAPPINGS

		// Gets or creates the state object for a given key
		KeyInfo_* GetInfoForKey(KeyCode keyCode, bool incRef);
		// Releases a reference to a keycode.  This implies decrementing the reference count
		// and possibly removing the corresponding key info object -- and key state pointer from the
		// states vector below
		void ReleaseKey(KeyCode code);

		// DATA
		std::unordered_map<std::string, size_t> m_actionNameMap;
		std::vector<ActionMapping_>  m_actions;

		// Store of key nodes.  Pointers can be saved because maps preserve references
		// The second array is used to store KeyState pointers for querying the input
		std::unordered_map<unsigned int, KeyInfo_>  m_keyInfoMap;
		std::vector<KeyState*> m_keyStates;
	};


}