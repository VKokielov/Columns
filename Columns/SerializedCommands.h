#pragma once
#include "CommandInterface.h"
#include "Bytestream.h"

#include <memory>
#include <cinttypes>

namespace geng::serial
{
	class ICommandDelta
	{
	public:
		virtual ~ICommandDelta() = default;

		// Compute the delta between the previous and the current state of a command
		virtual void OnCommand(const ICommand& nextCommand) = 0;

		virtual bool HasDelta() const = 0;
		// Write a delta to a buffer.
		virtual bool Write(IWriteStream* pStream) = 0;

		// Read a delta from a buffer
		virtual bool Read(IReadStream* pStream) = 0;
		// Apply a delta to a command
		virtual void ApplyTo(ICommand& rCommand) const = 0;
	};

	class ISerializableCommand : public ICommand
	{
	public:
		virtual ICommandDelta*
			AllocateDeltaObject() = 0;
	};

}