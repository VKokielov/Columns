#pragma once

#include "UIEvent.h"
#include "UISubscription.h"
#include "LeveledHashmap.h"
#include <unordered_map>
#include <unordered_set>
// Use std::optional for access to formalize the "index stack" used here
#include <optional>

namespace geng::ui::impl
{
	// This class helps the UI elements and engine manage subscriptions
	struct EventDescription
	{
		const UIEvent* pEvent;
		StringID eventId;
		UIAddress uiAddress;
		// AgentID -- subscriptions with this agent are not called
		AgentID agentId;


	};


	class UISubscriptionHelper
	{
	private:
		class SubHelperEnvelope : public UIEventEnvelope
		{
			// Allowing the subscription helper class itself to set the various 
			// envelope properties
			friend class UISubscriptionHelper;
		};

		using SubIndex = size_t;

		struct SubKey
		{
			StringID eventId;
			UIAddress uiAddress;

			template<size_t depth>
			struct Getter;

			template<>
			struct Getter<0>
			{
				static StringID Get(const SubKey& key)
				{
					return key.eventId;
				}
			};

			template<>
			struct Getter<1>
			{
				static ElementID Get(const SubKey& key)
				{
					return key.uiAddress.elementId;
				}
			};

			template<>
			struct Getter<2>
			{
				static SubelementID Get(const SubKey& key)
				{
					return key.uiAddress.subElementId;
				}
			};
		};

		struct Subscription_ 
		{ 
			SubKey key;
			SubscriptionID subId;
			SubUserID subUserId;
			AgentID agentId;
			bool requestGeneric;  // Request an event in the general form
			std::shared_ptr<IUIEventSubscriber> pSubscriber;
		};

		struct SubList
		{
			std::unordered_set<SubIndex>  subs;
		};

	public:
		bool AddSubscription(const SubscribeRequest& subRequest,
			const std::shared_ptr<IUIEventSubscriber>& pSubscriber,
			SubscriptionID subId);
		bool RemoveSubscription(SubscriptionID subId);
		bool BroadcastEvent(const EventDescription& eventDesc);

	private:
		SubIndex AllocateIndex()
		{
			SubIndex rvIndex;
			if (m_indexStack.empty())
			{
				rvIndex = m_subVector.size();
				m_subVector.emplace_back();
			}
			else
			{
				rvIndex = m_indexStack.back();
				m_indexStack.pop_back();
			}

			return rvIndex;
		}

		void FreeIndex(SubIndex subIndex)
		{
			if (subIndex == m_subVector.size() - 1)
			{
				m_subVector.pop_back();
			}
			else
			{
				m_indexStack.push_back(subIndex);
			}

			if (m_subVector.size() == m_indexStack.size())
			{
				// This means that subVector only contains unallocated
				// optionals
				m_subVector.clear();
				m_indexStack.clear();
			}

		}

		lib::LeveledHashmap<SubList, StringID, ElementID, SubelementID>
			m_subLocatorMap;
		std::unordered_map<SubscriptionID, SubIndex> m_subPointerMap;
		std::vector<std::optional<Subscription_>> m_subVector;
		// To avoid shoving elements around we use a free-index stack
		std::vector<SubIndex> m_indexStack;
	};



}