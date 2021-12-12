#pragma once

#include "SerializedCommands.h"
#include "Filestream.h"

#include <unordered_map>

namespace geng::serial
{

	enum class FilePlaybackStatus
	{
		FileError,
		FilePlaybackOpen,
		FilePlaybackComplete // only upon receiving an ending packet and arriving at the corresponding frame
	};

	// This is not a random-seek set of streams
	// If the Update() functions in this class submit a frame out of order, the 
	// whole thing will fail
	class FileCommandReader
	{
	private:

		class FileCommandStream : public ICommandStream
		{
		public:
			FileCommandStream(FileCommandReader& rOwner,
							const std::shared_ptr<ISerializableCommand>& pCommand);
			bool UpdateOnFrame(unsigned long frameIndex) override;

			bool ReadDelta(FileReadStream& rStream);
			bool ApplyDelta();
		private:
			FileCommandReader& m_rOwner;
			// From outside
			std::shared_ptr<ISerializableCommand>  m_pCommand;
			std::unique_ptr<ICommandDelta>  m_pDeltaHolder;
		};

		struct CommandStreamInfo
		{
			std::shared_ptr<FileCommandStream>  cmdStream;
		};

	public:
		// The file is opened from outside and this class takes ownership
		// The list of commands is searched and matched to each key
		// present in the file.  The match must be a bijection; there is no way
		// for a frame to be processed unless all commands present in the file
		// have counterparts 

		FileCommandReader(FileUPtr&& pFile,
						  const std::vector<std::shared_ptr<ISerializableCommand> >&
								commandList);

		std::shared_ptr<ICommandStream> GetCommandStream(const char* pCommandKey);

		bool IsValid() const { return m_valid; }
		const std::string& GetError() const { return m_error; }

		bool IsWrappedUp() const {
			return m_filePBStatus == FilePlaybackStatus::FilePlaybackComplete;
		}

	private:
		unsigned long CurrentFrame() const { return m_currentFrame; }
		// Set the frame and load the deltas
		bool SetFrame(unsigned long curFrame);

		bool LoadNextFrame();

		FileReadStream m_fileStream;
		bool  m_valid{ false };

		// There are two ways this variable is set to "complete"
		// One is the LoadNextFrame() function, which sets it if it encounters an "end" signal
		// for the same frame.
		// Another is the calling SetFrame() function, which sets it when it reaches a frame 
		// with an explicitly zero-set of deltas

		FilePlaybackStatus  m_filePBStatus{ FilePlaybackStatus::FileError };
		std::string m_error;

		// Map of command keys to command vector entries
		std::unordered_map<std::string, uint32_t>  m_commandStreamMap;
		std::vector<size_t> m_nextDeltas;
		std::vector<CommandStreamInfo>  m_commandStreams;

		unsigned long m_currentFrame{ 0 };
		unsigned long m_nextFrame{ 0 };
	};




}