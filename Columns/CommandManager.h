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
		Playback,
		Ended
	};

	class CommandDesc
	{
	public:
		CommandDesc(
			const std::shared_ptr<serial::ISerializableCommand>& pCommand,
			const FactorySharedPtr<ICommandStream>& pStreamFactory)
			:m_pCommand(pCommand),
			m_pStreamFactory(pStreamFactory)
		{

		}

	private:
		std::shared_ptr<serial::ISerializableCommand> m_pCommand;
		// The command stream factory gives the type of the stream that will produce commands
		// This is used only when not in playback mode
		FactorySharedPtr<ICommandStream> m_pStreamFactory;
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
			const std::vector<CommandDesc>& cmdList);

		// In read mode, unsubscribe the commands from the reader in order
		// to free up circular refs
		~CommandManager();

		void OnFrame(unsigned long frameIndex);
		void EndFrame();
		void EndSession();

		PlaybackMode GetPBMode() const { return m_playbackMode; }

		bool IsEndOfPlayback() const;
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
				|| mode == PlaybackMode::Record;
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