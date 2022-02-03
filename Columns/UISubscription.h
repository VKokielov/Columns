#pragma once

#include "UIEvent.h"
#include "UIDatatree.h"
#include <memory>

namespace geng::ui
{
	enum class SubscriptionStatus
	{
		OK,
		NoSuchElement,
		InvalidSubelement,
		InvalidEvent,
		InvalidSubscriptionID
	};

	struct SubscribeRequest
	{
		// [in]
		ElementID elementId;
		SubelementID subelementId;
		EventID eventId;
		SubUserID subUserId;
		AgentID agentId;
		bool requestGeneric;  // Request an event in the general form
		// [out]
		SubscriptionStatus out_status;
		SubscriptionID out_subId;
	};

	struct UnsubscribeRequest
	{
		// [in]
		SubscriptionID  subId;
		// [out]
		SubscriptionStatus out_status;
	};

	class IUISubscriber
	{
	public:
		virtual ~IUISubscriber() = default;
		virtual bool OnEvent(const UIEventEnvelope& rEvent) = 0;
	};

	class IUISubscriptionManager : public IUIManager
	{
	public:
		virtual bool Subscribe(const std::shared_ptr<IUISubscriber>& pSubscriber,
			SubscribeRequest* pRequests,
			size_t nRequests) = 0;
		virtual bool Unsubscribe(UnsubscribeRequest* pRequests,
			size_t nRequests) = 0;
	};


}