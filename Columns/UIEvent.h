#pragma once

#include "UIBase.h"
#include "IDataTree.h"

namespace geng::ui
{
	class UIEvent { };

	class UIEventEnvelope
	{
	public:

		// Ideally this value defines the other coordinates too
		SubUserID GetUserSubID() const { return m_userSubId; }

		StringID GetEventStringID() const { return m_eventId; }
		const UIAddress& GetAddress() const { return m_uiAddress; }
		SubscriptionID GetSubscriptionID() const { return m_subId; }

		// If the generic form is requested and available, it will be supplied here
		const UIEvent* GetEvent() const { return m_event; }
		std::shared_ptr<data::IDatum> GetGenericEvent() const { return m_genericEvent; }

	protected:		
		void SetEventStringID(StringID eventId)
		{
			m_eventId = eventId;
		}

		void SetElementID(ElementID elementId)
		{
			m_uiAddress.elementId = elementId;
		}

		void SetSubelementID(SubelementID subElementId)
		{
			m_uiAddress.subElementId = subElementId;
		}

		void SetEventObj(const UIEvent* pEvent)
		{
			m_event = pEvent;
		}

		void SetGenericEventObj(const std::shared_ptr<data::IDatum>& genericEvent)
		{
			m_genericEvent = genericEvent;
		}

		void SetSubscriptionID(SubscriptionID subId)
		{
			m_subId = subId;
		}

		void SetSubUserID(SubUserID subUserId)
		{
			m_userSubId = subUserId;
		}

	private:
		SubUserID m_userSubId;
		StringID  m_eventId;
		UIAddress  m_uiAddress;
		SubscriptionID m_subId;
		const UIEvent* m_event{ nullptr };
		std::shared_ptr<data::IDatum> m_genericEvent;
	};

	struct EventInfo
	{
		StringID eventStringID;
	};


	class IUIEventManager : public IUIManager
	{
	public:
		virtual const EventInfo* GetEvent(const char* pEventName) = 0;
	};

}