#pragma once

#include "BaseCommand.h"
#include "ActionTranslator.h"
#include "FactoryImpl.h"

namespace geng
{

	// An "action" command is represented by a simple on/off state and an identical delta
	using ActionCommand = TypedCommand<bool, bool>;

	// An action command stream reads the value of an action from an action translator
	// (which should have been updated beforehand) and sets the state of the command based
	// on this value

	class ActionCommandStreamArgs
	{
	public:
		ActionCommandStreamArgs(const std::shared_ptr<ActionCommand>& pCommand,
			const std::shared_ptr<ActionTranslator>& pTranslator,
			unsigned long msPerFrame,
			ActionID actionId)
			:m_pCommand(pCommand),
			m_pTranslator(pTranslator),
			m_msPerFrame(msPerFrame),
			m_actionId(actionId)
		{ }

	private:
		std::shared_ptr<ActionCommand>      m_pCommand;
		std::shared_ptr<ActionTranslator>   m_pTranslator;
		unsigned long m_msPerFrame{ 0 };
		ActionID m_actionId;

		friend class ActionCommandStream;
	};

	class ActionCommandStream : public ICommandStream
	{
	public:
		bool UpdateOnFrame(unsigned long frameIndex) override
		{
			ActionState curActionState = m_args.m_pTranslator->GetActionState(m_args.m_actionId);

			bool curCommandState = GetCommandState(frameIndex * m_args.m_msPerFrame, curActionState);

			/*
			if (curActionState == ActionState::On)
			{
				fprintf(stderr, "Curactionstate: command state %d\n", curCommandState);
			}
			*/

			if (frameIndex == 1 || 
				m_prevState != curCommandState)
			{
				// This will also invoke relevant listeners
			//	fprintf(stderr, "Setting command stream %lu to %d\n", m_args.m_actionId, curActionState);
				m_args.m_pCommand->SetState(curCommandState);
			}
			m_prevState = curCommandState;
			return true;
		}

	protected:
		ActionCommandStream(const ActionCommandStreamArgs& args)
			:m_args(args)
		{

		}

		// This maps the current command state to the current action state
		// The default implementation can be overridden by derived classes
		virtual bool GetCommandState(unsigned long simTime, 
									 ActionState actionState)
		{
			return actionState == ActionState::On;
		}

	private:
		ActionCommandStreamArgs  m_args;

		bool m_prevState{ false };
	};

	class ThrottledActionCommandStream : public ActionCommandStream
	{
	public:
		ThrottledActionCommandStream(const ActionCommandStreamArgs& args, 
				unsigned long throttlePeriod)
			:ActionCommandStream(args),
			m_msThrottlePeriod(throttlePeriod)
		{ }

	protected:
		bool GetCommandState(unsigned long simTime,
			ActionState actionState) override
		{
			bool rv{ false };
			if (actionState == ActionState::On)
			{
				if (m_prevActionState == ActionState::Off
					|| simTime >= m_nextOnTime)
				{
					m_nextOnTime = simTime + m_msThrottlePeriod;
					rv = true;
				}
			}

			m_prevActionState = actionState;

			return rv;
		}

	private:
		ActionState m_prevActionState{ ActionState::Off };
		unsigned long m_msThrottlePeriod{ 0 };
		unsigned long m_nextOnTime{ 0 };
	};
}