#include "UIImplSubscriptionHelper.h"

namespace impl_ns = geng::ui::impl;

bool impl_ns::UISubscriptionHelper::AddSubscription(const geng::ui::SubscribeRequest& subRequest,
	const std::shared_ptr<geng::ui::IUIEventSubscriber>& pSubscriber,
	geng::ui::SubscriptionID subId)
{
	SubIndex subIndex = AllocateIndex();
	auto insById = m_subPointerMap.emplace(subId, subIndex);
	if (!insById.second)
	{
		// Two entries for one subscription
		return false;
	}

	Subscription_ subInfo;
	subInfo.key.eventId = subRequest.eventStringId;
	subInfo.key.uiAddress = subRequest.uiAddress;
	subInfo.subId = subId;
	subInfo.subUserId = subRequest.subUserId;
	subInfo.agentId = subRequest.agentId;
	subInfo.requestGeneric = subRequest.requestGeneric;
	subInfo.pSubscriber = pSubscriber;

	auto insByData
		= lib::map_helpers::Insert(m_subLocatorMap, subInfo.key);
	insByData.first->second.subs.insert(subIndex);

	// Add the sub to the vector
	m_subVector[subIndex].emplace(subInfo);

	return true;
}

bool impl_ns::UISubscriptionHelper::RemoveSubscription(geng::ui::SubscriptionID subId)
{
	// Remove a subscription
	auto itSub = m_subPointerMap.find(subId);

	if (itSub == m_subPointerMap.end())
	{
		return false;
	}

	SubIndex subIndex = itSub->second;

	// The sub ID is known to be valid since it was in the pointer map.
	// We therefore freely assume that the vector element is valid
	Subscription_& rSub = *(m_subVector[subIndex]);

	// Remove the sub from the locator
	auto pSubList = lib::map_helpers::Find(m_subLocatorMap, rSub.key);
	pSubList->subs.erase(subIndex);
	if (pSubList->subs.empty())
	{
		// TODO: It may be a good idea to optimize this by separating erasure
		// from sub removal
		lib::map_helpers::Erase(m_subLocatorMap, rSub.key, true);
	}

	// Inform the caller
	rSub.pSubscriber->OnSubRemoved(rSub.subUserId);

	// Erase from the data structures
	m_subPointerMap.erase(itSub);
	m_subVector[subIndex].reset();

	FreeIndex(subIndex);

	return true;
}

bool impl_ns::UISubscriptionHelper::BroadcastEvent(const EventDescription& eventDesc)
{
	SubKey subKey;
	subKey.eventId = eventDesc.eventId;
	subKey.uiAddress = eventDesc.uiAddress;
	auto pSubList = lib::map_helpers::Find(m_subLocatorMap, subKey);
	if (!pSubList)
	{
		// No one was subscribed to this event
		return false;
	}

	SubHelperEnvelope envelope;
	envelope.SetElementID(eventDesc.uiAddress.elementId);
	envelope.SetSubelementID(eventDesc.uiAddress.subElementId);
	envelope.SetEventStringID(eventDesc.eventId);
	envelope.SetEventObj(eventDesc.pEvent);

	for (SubIndex subIndex : pSubList->subs)
	{
		const Subscription_& rSub = *(m_subVector[subIndex]);

		if (rSub.agentId != eventDesc.agentId)
		{
			envelope.SetSubscriptionID(rSub.subId);
			envelope.SetSubUserID(rSub.subUserId);
			rSub.pSubscriber->OnEvent(envelope);
		}
	}

	return true;
}