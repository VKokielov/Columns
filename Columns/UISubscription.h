#pragma once

#include "UIEvent.h"
#include "IDataTree.h"
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
		StringID eventStringId;
		UIAddress uiAddress;
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
		virtual bool OnFrameStart() = 0;
		virtual bool OnEvent(const UIEventEnvelope& rEvent) = 0;
		virtual bool OnFrameEnd() = 0;
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