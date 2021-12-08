#pragma once

#include "BaseCommand.h"
#include "FactoryImpl.h"

namespace geng
{
	// The integral value command takes a shared_ptr to a value and reads that value
	// into the command state

	// The idea is to get this same value during playback from the file
	
	template<typename T>
	using SharedValueCommand = TypedCommand<T, T>;

	template<typename T>
	class SharedValueCommandStream : public ICommandStream
	{
	public:

		SharedValueCommandStream(const std::shared_ptr<SharedValueCommand<T> >& pCommand,
								const std::shared_ptr<const T>& pValue)
			:m_pCommand(pCommand),
			m_pValue(pValue),
			m_firstFrame(true)
		{ }

		bool UpdateOnFrame(unsigned long frameIndex) override
		{
			// Update the command
			const auto& rValue = *m_pValue;
			if (m_firstFrame || m_pCommand->GetState() != rValue)
			{
				m_pCommand->SetState(rValue);
				m_firstFrame = false;
			}
			return true;
		}
	private:
		std::shared_ptr< SharedValueCommand<T> > m_pCommand;
		std::shared_ptr<const T> m_pValue;
		bool m_firstFrame;
	};

}