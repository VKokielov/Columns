#include "FileCommandWriter.h"
#include <limits>

geng::serial::FileCommandWriter::Command_::Command_(const std::shared_ptr<ISerializableCommand>& pCommand_)
	:pCommand(pCommand_)
{
	pDelta.reset(pCommand->AllocateDeltaObject());
}

geng::serial::FileCommandWriter::FileCommandWriter(FileUPtr&& pFile,
	const std::vector<std::shared_ptr<ISerializableCommand> >&
	commandList)
	:m_fileStream(std::move(pFile))
{
	// Write out the header to the file and construct the vector of commands
	// The header is a sequence of command keys terminated by a zero-length
	// string

	for (auto& pCommand : commandList)
	{
		std::string cmdKey{ pCommand->GetKey() };

		if (cmdKey.empty() || cmdKey.size() > std::numeric_limits<uint16_t>::max())
		{
			// Ignore this command
			continue;
		}
		
		uint16_t keySize = (uint16_t)(cmdKey.size());
		m_fileStream.Write(&keySize, sizeof(keySize));
		m_fileStream.Write(cmdKey.c_str(), keySize);
		m_commands.emplace_back(pCommand);
	}

	const uint16_t endMarker{ 0 };
	m_fileStream.Write(&endMarker, sizeof(endMarker));
}

void geng::serial::FileCommandWriter::BeginFrame(unsigned long currentFrame)
{
	m_frameChanges = 0;
	m_currentFrame = currentFrame;
}
void geng::serial::FileCommandWriter::OnCommandChanged(SubID subId,
	const ICommand& cmd)
{
	Command_& cmdObject = m_commands[subId];

	if (cmdObject.lastUpdatedFrame == -1 ||
		cmdObject.lastUpdatedFrame < m_currentFrame)
	{
		cmdObject.pDelta->OnCommand(*cmdObject.pCommand.get());

		if (cmdObject.pDelta->HasDelta())
		{
			cmdObject.lastUpdatedFrame = m_currentFrame;
			++m_frameChanges;
		}
	}
}


void geng::serial::FileCommandWriter::EndFrame()
{
	if (m_frameChanges > 0)
	{
		SaveFrame();
	}
}

void geng::serial::FileCommandWriter::SaveFrame()
{
	// 1. Frame number
	uint32_t frameNumber = (uint32_t)(m_currentFrame);
	m_fileStream.Write(&frameNumber, sizeof(frameNumber));

	// Number of deltas (== number of changes)
	uint32_t changeCount = (uint32_t)(m_frameChanges);
	m_fileStream.Write(&changeCount, sizeof(changeCount));

	// Write any deltas
	for (size_t cmdId = 0; cmdId < m_commands.size(); ++cmdId)
	{
		if (m_commands[cmdId].lastUpdatedFrame == m_currentFrame)
		{
			uint32_t commandIndex = (uint32_t)cmdId;
			m_fileStream.Write(&commandIndex, sizeof(commandIndex));
			m_commands[cmdId].pDelta->Write(&m_fileStream);
		}
	}
}