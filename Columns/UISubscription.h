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

	class IUIEngineSubscriber
	{
	public:
		virtual ~IUIEngineSubscriber() = default;

		virtual void OnFrameBegin() = 0;
		virtual void OnFrameEnd() = 0;
		virtual void OnElementCreated(ElementID elementId) = 0;
		virtual void OnElementDestroyed(ElementID elementId) = 0;
	};

	class IUIEventSubscriber
	{
	public:
		virtual ~IUIEventSubscriber() = default;
		// When a subscription is removed manually or because the referenced element
		// no longer exists, this function is called
		virtual bool OnSubRemoved(SubUserID userId) = 0;
		virtual bool OnEvent(const UIEventEnvelope& rEvent) = 0;
	};

	class IUISubscriptionManager : public IUIManager
	{
	public:
		virtual bool Subscribe(const std::shared_ptr<IUIEventSubscriber>& pSubscriber,
			SubscribeRequest* pRequests,
			size_t nRequests) = 0;
		virtual bool Unsubscribe(UnsubscribeRequest* pRequests,
			size_t nRequests) = 0;
	};


}