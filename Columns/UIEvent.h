#pragma once

#include "UIElements.h"

namespace geng::ui
{
	enum class EventType
	{
		ActionEvent,
		ElementChangeEvent,
		ListChangeEvent,
		DictChangeEvent,
		UserEvent
	};

	class UIEvent
	{
	public:
		EventType GetEventType() const;

		// Ideally this value defines the other coordinates too
		SubUserID GetUserSubID() const;

		EventID GetEventID() const;
		const UIAddress& GetAddress() const;
		SubscriptionID GetSubscriptionID() const;

		// If the generic form is requested, it will be supplied here
		const data::IDatum* GetGeneric() const;
	};

	class UIEventEnvelope;

	struct EventInfo
	{
		const char* pName;
		EventID eventId;
		EventType eventType;
	};


	class IUIEventManager : public IUIManager
	{
	public:
		virtual const EventInfo* GetEvent(const char* pEventName) = 0;
	};

}