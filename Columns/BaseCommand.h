#pragma once

#include "SerializedCommands.h"
#include <string>
#include <vector>

namespace geng
{
	// Default encode, decode, compute and apply functions
	template<typename State>
	bool ComputeCommandDiff(const State& prevState, const State& curState,
		State& outDelta)
	{
		if (prevState == curState)
		{
			return false;
		}

		// The delta is simply the new state
		outDelta = curState;
		return true;
	}

	template<typename State>
	void ApplyCommandDiff(State& curState, const State& delta)
	{
		curState = delta;
	}

	template<typename Data>
	bool EncodeData(serial::IWriteStream* pWriteStream, Data&& data)
	{
		return pWriteStream->Write(&data, sizeof(Data)) == sizeof(Data);
	}

	template<typename Data>
	bool DecodeData(serial::IReadStream* pReadStream, Data& data)
	{
		return pReadStream->Read(&data, sizeof(Data)) == sizeof(Data);
	}


	template<typename State, typename StateDelta = State>
	class TypedCommand : public serial::ISerializableCommand
	{
	private:
		struct Subscriber_
		{
			SubID id;
			std::shared_ptr<ICommandListener> pListener;

			Subscriber_(SubID id_, const std::shared_ptr<ICommandListener>& pListener_)
				:id(id_),
				pListener(pListener_)
			{ }

			void Dispatch(const ICommand& cmd) const
			{
				pListener->OnCommandChanged(id, cmd);
			}
		};

		class Delta_ : public serial::ICommandDelta
		{
		public:
			void OnCommand(const ICommand& nextCommand) override
			{
				// Compute the delta using the two states, previous and current
				const TypedCommand& typedCommand = static_cast<const TypedCommand&>(nextCommand);

				const State& nextState = typedCommand.GetState();

				m_hasDelta = ComputeCommandDiff(nextState, m_curState, m_stateDelta);
				m_curState = nextState;
			}

			bool HasDelta() const override
			{
				return m_hasDelta;
			}
			bool Write(serial::IWriteStream* pStream) override
			{
				if (!m_hasDelta)
				{
					return false;
				}
				return EncodeData(pStream, m_stateDelta);
			}
			bool Read(serial::IReadStream* pStream) override
			{
				return DecodeData(pStream, m_stateDelta);
			}
			void ApplyTo(ICommand& curCommand) const override
			{
				TypedCommand& typedCommand = static_cast<TypedCommand&>(curCommand);
				ApplyCommandDiff(typedCommand.m_commandState,
					m_stateDelta);
			}
		private:
			bool m_hasDelta{ false };
			State m_curState;
			StateDelta m_stateDelta;
		};

	public:
		TypedCommand(const char* pKey)
			:m_key(pKey)
		{ }

		const char* GetKey() const override
		{
			return m_key.c_str();
		}

		void Subscribe(const std::shared_ptr<ICommandListener>& pListener,
			SubID subId) override
		{
			for (auto& rListener : m_vCommandListeners)
			{
				if (rListener.pListener == pListener)
				{
					rListener.id = subId;
					return;
				}
			}

			m_vCommandListeners.emplace_back(subId, pListener);
		}
		
		void Unsubscribe(const std::shared_ptr<ICommandListener>& pListener) override
		{
			size_t idx = 0;
			for (auto& rListener : m_vCommandListeners)
			{
				if (rListener.pListener == pListener)
				{
					m_vCommandListeners.erase(m_vCommandListeners.begin() + idx);
					return;
				}
				++idx;
			}
		}

		void Reset() override
		{
			m_commandState = State{};
		}

		serial::ICommandDelta*
			AllocateDeltaObject() override
		{
			return new Delta_();
		}

		const State& GetState() const { return m_commandState; }

		void SetState(const State& newState)
		{
			if (newState != m_commandState)
			{
				m_commandState = newState;

				for (const auto& commandListener : m_vCommandListeners)
				{
					commandListener.Dispatch(*this);
				}
			}
		}

	private:
		std::string  m_key;
		State m_commandState{};
		std::vector<Subscriber_>
			m_vCommandListeners;
	};
}