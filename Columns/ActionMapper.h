#pragma once

#include <unordered_map>
#include <vector>
#include <cinttypes>
#include <optional>
#include <string>
#include <type_traits>
#include <iterator>
#include <unordered_set>

#include "IInput.h"
#include "BaseGameComponent.h"

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
		Initial,  // 0
		Off,  // 1
		Starting, // 2
		On, // 3
		Ending  // 4
	};

	class ActionMapper : public BaseGameComponent,
		                 public IFrameListener
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
		ActionMapper(const char* pInputName);

		IFrameListener* GetFrameListener() override;

		bool Initialize(const std::shared_ptr<IGame>& pGame) override;
		void OnFrame(IFrameManager* pManager) override;

		// Action management 

		// This function will return an existing action ID if the action already exists
		ActionID CreateAction(const char* pName);
		// Get an existing action
		ActionID GetAction(const char* pName) const;

		template<typename Iter>
		bool MapAction(ActionID actionId, Iter bCodes, Iter eCodes)
		{
			static_assert(std::is_same_v< std::remove_reference_t<decltype(*bCodes)>, KeyCode>,
				"bCodes: expecting KeyCode container's iterator");
			static_assert(std::is_same_v<std::remove_reference_t<decltype(*eCodes)>, KeyCode>,
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
				m_usedKeys.emplace(keyCode);
				++bCodes;
			}

			return true;
		}

		bool MapAction(ActionID actionId, KeyCode code)
		{
			// Old C trick -- one-element array
			return MapAction(actionId, &code, (&code) + 1);
		}

		// Action states
		// The mouse state paramter is simply passed through

		ActionState GetActionState(ActionID actionId) const
		{
			if (actionId < 0 || actionId >= (ActionID)m_actions.size())
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

		std::string m_inputName;

		// DATA
		std::unordered_map<std::string, size_t> m_actionNameMap;
		std::vector<ActionMapping_>  m_actions;

		// Store of key nodes.  Pointers can be saved because maps preserve references
		// The second array is used to store KeyState pointers for querying the input
		std::unordered_map<unsigned int, KeyInfo_>  m_keyInfoMap;
		std::vector<KeyState*> m_keyStates;

		std::unordered_set<KeyCode>  m_usedKeys;
		std::shared_ptr<IInput> m_pInput;
		MouseState m_mouseState;
	};

	class ActionWrapper
	{
	public:
		ActionWrapper() = delete;

		bool Triggered() const { return m_triggered; }

	protected:
		// Default implementation: action in "starting" and "on" state
		void UpdateState(const ActionMapper& rMapper)
		{
			ActionState state = GetMyActionState(rMapper);
			SetTriggered(state == ActionState::On || state == ActionState::Starting);
		}

		ActionWrapper(const char* pName, ActionMapper& rMapper)
		{
			m_id = rMapper.CreateAction(pName);
		}

		void SetTriggered(bool isTriggered)
		{
			m_triggered = isTriggered;
		}
		ActionState GetMyActionState(const ActionMapper& rMapper)
		{
			return rMapper.GetActionState(m_id);
		}

		ActionID GetID() const { return m_id; }

	private:
		ActionID m_id{ INVALID_ACTION };
		bool m_triggered{ false };
	};

	class ThrottledActionWrapper : public ActionWrapper
	{
	public:
		ThrottledActionWrapper(const char* pactionName, unsigned int throttlePeriod,
			ActionMapper& rMapper)
			:ActionWrapper(pactionName, rMapper),
			m_throttlePeriod(throttlePeriod)
		{
			
		}

		void UpdateState(const ActionMapper& rMapper, unsigned long simTime)
		{
			ActionState state = GetMyActionState(rMapper);
			bool isStarting = state == ActionState::Starting;
			if (m_wasStarting && isStarting)
			{
				fprintf(stderr, "Starting twice!\n");
			}
			m_wasStarting = isStarting;

			if (state == ActionState::Starting
			  || ((state == ActionState::On) && (m_throttleStart + m_throttlePeriod <= simTime)))
			{
//				fprintf(stderr, "setting action %d to true with state %d\n", GetID(), state);
				m_throttleStart = simTime;
				SetTriggered(true);
			}
			else
			{
				/*
				if (Triggered())
				{
					fprintf(stderr, "setting action %d to false\n", GetID());
				}
				*/
				
				SetTriggered(false);
			}
		}
	private:
		bool m_wasStarting{ false };
		unsigned int  m_throttlePeriod;
		unsigned long  m_throttleStart{ 0 };
	};


}