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

	struct ActionMapping
	{
		// This is an OR of ANDs expression
		// It expresses any truth function on the keys (any key not in the AND vector
		// is complemented implicitly)
		std::vector<std::vector<KeyCode> > keyGroups;
	};

	class IActionMappingListener
	{
	public:
		virtual ~IActionMappingListener() = default;

		virtual void OnMapping(ActionID actionId, const ActionMapping& mapping) = 0;
	};

	class ActionMapper : public BaseGameComponent
	{
	private:

		struct ActionDef_
		{
			ActionID id{ INVALID_ACTION };
			std::string actionName;
			ActionMapping actionMapping;

			ActionDef_(ActionID id_, const std::string& actionName_)
				:id(id_),
				actionName(actionName_)
			{ }

			bool HasMapping(const std::vector<KeyCode>& mapping) const
			{
				for (const auto& curMapping : actionMapping.keyGroups)
				{
					if (mapping == curMapping)
					{
						return true;
					}
				}
				return false;
			}
		};

	public:
		ActionMapper(const char* pName);

		// Action management 

		// This function will return an existing action ID if the action already exists
		ActionID CreateAction(const char* pName);
		// Get an existing action
		ActionID GetAction(const char* pName) const;

		bool ClearMapping(ActionID actionId);

		template<typename Iter>
		bool MapAction(ActionID actionId, Iter bCodes, Iter eCodes)
		{
			static_assert(std::is_same_v< std::remove_reference_t<decltype(*bCodes)>, KeyCode>,
				"bCodes: expecting KeyCode container's iterator");
			static_assert(std::is_same_v<std::remove_reference_t<decltype(*eCodes)>, KeyCode>,
				"eCodes: expecting KeyCode container's iterator");

			if (actionId < 0 || actionId >= m_actions.size() 
				|| bCodes == eCodes)
			{
				return false;
			}

			std::vector<KeyCode>  vMapping{ bCodes,eCodes };

			// Check to make sure this mapping/group does not identically match
			// a different, existing action
			// (For now do it the dumb way, by iterating through the actions)
			for (const auto& action : m_actions)
			{
				if (action.HasMapping(vMapping))
				{
					return false;
				}
			}

			ActionDef_& action = m_actions[actionId];
			action.actionMapping.keyGroups.emplace_back(std::move(vMapping));

			for (const auto& pListener : m_vMappingListeners)
			{
				pListener->OnMapping(actionId, action.actionMapping);
			}

			return true;
		}

		bool MapAction(ActionID actionId, KeyCode code)
		{
			// Old C trick -- one-element array
			return MapAction(actionId, &code, (&code) + 1);
		}

		void GetAllMappings(const std::shared_ptr<IActionMappingListener>& pListener);
		void AddMappingListener(const std::shared_ptr<IActionMappingListener>& pListener);

	private:
		friend class ActionTranslator;

		// KEY MAPPINGS
		std::string m_inputName;

		// DATA
		std::unordered_map<std::string, size_t> m_actionNameMap;
		std::vector<ActionDef_>  m_actions;
		std::vector<std::shared_ptr<IActionMappingListener> >
			m_vMappingListeners;
	};




}