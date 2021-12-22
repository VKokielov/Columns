#pragma once

#include "SerializedCommands.h"
#include "Filestream.h"
#include <vector>
#include <limits>

namespace geng::serial
{

	class FileCommandWriter : public ICommandListener
	{
	private:
		static constexpr unsigned long INITIAL_FRAME_TAG =
			std::numeric_limits<unsigned long>::max();

		struct Command_
		{
			std::shared_ptr<ISerializableCommand>  pCommand;
			std::shared_ptr<ICommandDelta>  pDelta;
			// Prevents changes from being applied twice
			unsigned long lastUpdatedFrame{ INITIAL_FRAME_TAG };

			Command_(const std::shared_ptr<ISerializableCommand>& pCommand_);
		};
	public:
		FileCommandWriter(FileUPtr&& pFile,
			const std::shared_ptr<IPacket>& pDescriptionPacket,
			const std::vector<std::shared_ptr<ISerializableCommand> >& commandList,
			const FileStreamHeader* pStreamHeader);

		// This is done thus strangely to avoid declaring the class enable_shared_from_this
		// simply for this one step
		static void SubscribeToCommands(const std::shared_ptr<FileCommandWriter>& pThis, 
			bool subscribe = true);
	
		// Must be called before the commands are updated
		void BeginFrame(unsigned long curFrame);
		void OnCommandChanged(SubID subId,
			const ICommand& cmd) override;
		// Must be called after the commands are updated
		void EndFrame();
		// Called at the end of the game
		void EndSession();

		bool IsValid() const {
			return m_valid;
		}

		const std::string& GetError() const {
			return m_error;
		}

		void Flush();

	private:
		void SaveFrame(bool lastFrame);

		FileWriteStream m_fileStream;
		bool m_hasChecksum{ false };

		std::vector<Command_>   m_commands;
		unsigned long m_currentFrame{ 0 };

		// We only write frames with at least one changed command
		unsigned int m_frameChanges{ 0 };

		bool m_valid{ false };
		std::string m_error;
	};

}