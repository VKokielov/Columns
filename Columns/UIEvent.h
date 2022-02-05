#pragma once

#include "UIBase.h"
#include "IDataTree.h"

namespace geng::ui
{
	enum class EventType
	{
		ActionEvent,
		ValueChangeEvent,
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

		StringID GetEventStringID() const;
		const UIAddress& GetAddress() const;
		SubscriptionID GetSubscriptionID() const;

		// If the generic form is requested and available, it will be supplied here
		const data::IDatum* GetGeneric() const;
	};

	class UIEventEnvelope;

	struct EventInfo
	{
		StringID eventStringID;
		EventType eventType;
	};


	class IUIEventManager : public IUIManager
	{
	public:
		virtual const EventInfo* GetEvent(const char* pEventName) = 0;
	};

}