#pragma once

#include "ActionTranslator.h"

namespace geng
{
	class ActionWrapper
	{
	public:
		ActionWrapper() = delete;

		bool Triggered() const { return m_triggered; }
		const std::string& GetName() const { return m_name; }

	protected:
		// Default implementation: action in "starting" and "on" state
		void UpdateState(const ActionTranslator& rTranslator)
		{
			ActionState state = GetMyActionState(rTranslator);
			SetTriggered(state == ActionState::On);
		}

		ActionWrapper(const char* pName, ActionMapper& rMapper)
		{
			m_name = pName;
			m_id = rMapper.GetAction(pName);
		}

		void SetTriggered(bool isTriggered)
		{
			m_triggered = isTriggered;
		}
		ActionState GetMyActionState(const ActionTranslator& rTranslator)
		{
			return rTranslator.GetActionState(m_id);
		}

		ActionID GetID() const { return m_id; }


	private:
		std::string m_name;
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

		void UpdateState(const ActionTranslator& rTranslator, unsigned long simTime)
		{
			ActionState state = GetMyActionState(rTranslator);

			bool isOn = state == ActionState::On;

			if (isOn 
				&& (!m_wasOn
					|| (m_throttleStart + m_throttlePeriod <= simTime)))
			{
				//				fprintf(stderr, "setting action %d to true with state %d\n", GetID(), state);
				m_throttleStart = simTime;
				SetTriggered(true);
			}
			else
			{
				SetTriggered(false);
			}
			m_wasOn = isOn;
		}
	private:
		bool m_wasOn{ false };
		unsigned int  m_throttlePeriod;
		unsigned long  m_throttleStart{ 0 };
	};
}