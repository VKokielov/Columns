#pragma once

#include <string>
#include <memory>

namespace geng
{
	class ICommand;

	using SubID = uint64_t;

	class ICommandListener
	{
	public:
		virtual ~ICommandListener() = default;
		virtual void OnCommandChanged(SubID subId,
			const ICommand& cmd) = 0;
	};

	class ICommand
	{
	public:
		virtual const char* GetKey() const = 0;
		virtual void Subscribe(const std::shared_ptr<ICommandListener>& pListener,
			SubID subISd) = 0;
		virtual void Unsubscribe(const std::shared_ptr<ICommandListener>& pListener) = 0;

		virtual ~ICommand() = default;
	};

	// A single command stream represents a sequence of per-frame updates
	// to a command.  
	class ICommandStream
	{
	public:
		// Set the current frame and update the associated command
		virtual bool UpdateOnFrame(unsigned long frameIndex) = 0;
		
		virtual ~ICommandStream() = default;

	};

	// TODO:  Does this really belong here?  It would be much better in a "preheader" for
	// CommandManager
	class ICommandStreamArgs
	{
	public:
		virtual ~ICommandStreamArgs() = default;
	};

}