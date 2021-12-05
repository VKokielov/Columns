#pragma once

#include "IFactory.h"
#include "CommandInterface.h"
#include "SerializedCommands.h"

#include <vector>
#include <memory>

namespace geng
{
	// Forward declarations of reader and writer
	namespace serial
	{
		class FileCommandReader;
		class FileCommandWriter;
	}

	enum class PlaybackMode
	{
		None,
		Record,
		Playback
	};

	class CommandDesc
	{
	public:
		CommandDesc(const char* pCommandKey,
			const std::shared_ptr<ICommandStreamArgs>& pStreamArgs,
			const FactorySharedPtr<serial::ISerializableCommand, const char*>& pCommandFactory,
			const FactorySharedPtr<ICommandStream, ICommandStreamArgs>& pStreamFactory)
			:m_commandKey(pCommandKey),
			m_pStreamArgs(pStreamArgs),
			m_pCommandFactory(pCommandFactory),
			m_pStreamFactory(pStreamFactory)
		{

		}

	private:
		// The command key identifies the command
		std::string m_commandKey;
		// The stream args specify how to initialize the stream when not in playback mode
		std::shared_ptr<ICommandStreamArgs> m_pStreamArgs;
		// The command factory gives the type of the command
		FactorySharedPtr<serial::ISerializableCommand, const char*> m_pCommandFactory;
		// The command stream factory gives the type of the stream that will produce commands
		// This is used only when not in playback mode
		FactorySharedPtr<ICommandStream, ICommandStreamArgs> m_pStreamFactory;

		friend class CommandManager;
	};

	class CommandList
	{
	public:

		void AddCommand(const CommandDesc& cmdDesc)
		{
			m_commands.push_back(cmdDesc);
		}

	private:
		std::vector<CommandDesc>  m_commands;

		friend class CommandManager;
	};


	class CommandManager
	{
	private:
		struct Command_
		{
			std::shared_ptr<serial::ISerializableCommand>  pCommand;
			std::shared_ptr<ICommandStream>  pStream;
		};
	public:

		CommandManager(PlaybackMode pbMode, 
			const char* pPBFile,
			const CommandList& cmdList);

		// In read mode, unsubscribe the commands from the reader in order
		// to free up circular refs
		~CommandManager();

		void OnFrame(unsigned long frameIndex);

		PlaybackMode GetPBMode() const { return m_playbackMode; }

	private:
		// Open the playback file and create an object to represent it
		bool OpenFile(const std::vector<std::shared_ptr<serial::ISerializableCommand> >& commandList);
		
		static bool HasFile(PlaybackMode mode)
		{
			return mode == PlaybackMode::Playback
				|| mode == PlaybackMode::Record;
		}

		static bool UserControl(PlaybackMode mode)
		{
			return mode == PlaybackMode::None
				|| mode == PlaybackMode::Playback;
		}

		bool m_valid{ false };
		std::string m_error;

		PlaybackMode m_playbackMode;
		std::string m_fileName;
		std::shared_ptr<serial::FileCommandReader>  m_pReader;
		std::shared_ptr<serial::FileCommandWriter>  m_pWriter;

		std::vector<Command_>  m_commands;

	};



}