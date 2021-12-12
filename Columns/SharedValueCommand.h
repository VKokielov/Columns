#pragma once

#include "BaseCommand.h"
#include "FactoryImpl.h"

namespace geng
{
	// The integral value command takes a shared_ptr to a value and reads that value
	// into the command state

	// The idea is to get this same value during playback from the file
	
	template<typename T>
	struct OptionalValue
	{
		bool hasVal{ false };
		T val;

		bool operator==(const OptionalValue<T>& other) const
		{
			return other.hasVal == hasVal
				&& other.val == val;
		}
	};

	template<typename T>
	using SharedValueCommand = TypedCommand<OptionalValue<T>, OptionalValue<T> >;

	// Need custom encoder and decoder for the optional
	template<typename T>
	bool EncodeData(serial::IWriteStream* pWriteStream, const OptionalValue<T>& data)
	{
		if (data.hasVal)
		{
			if (!EncodeData<bool>(pWriteStream, true))
			{
				return false;
			}
			return EncodeData<T>(pWriteStream, data.val);
		}
		
		return EncodeData<bool>(pWriteStream, true);
	}

	template<typename T>
	bool DecodeData(serial::IReadStream* pReadStream, OptionalValue<T>& data)
	{
		if (!DecodeData<bool>(pReadStream, data.hasVal))
		{
			return false;
		}

		if (data.hasVal)
		{
			if (!DecodeData<T>(pReadStream, data.val))
			{
				return false;
			}
		}

		return true;
	}

	template<typename T>
	struct SharedValue
	{
		// For what frame should this value exist?
		unsigned int targetFrame{ 0 };
		T val;
	};

	template<typename T>
	class SharedValueCommandStream : public ICommandStream
	{
	public:

		SharedValueCommandStream(const std::shared_ptr<SharedValueCommand<T> >& pCommand,
								const std::shared_ptr<const SharedValue<T> >& pValue)
			:m_pCommand(pCommand),
			m_pValue(pValue)
		{ }

		bool UpdateOnFrame(unsigned long frameIndex) override
		{
			OptionalValue<T> cmdState;

			if (frameIndex == m_pValue->targetFrame)
			{
				cmdState.hasVal = true;
				cmdState.val = m_pValue->val;
				m_pCommand->SetState(cmdState);
			}
			else
			{
				cmdState.hasVal = false;
				m_pCommand->SetState(cmdState);
			}
			return true;
		}
	private:
		std::shared_ptr< SharedValueCommand<T> > m_pCommand;
		std::shared_ptr<const SharedValue<T> > m_pValue;
	};

}