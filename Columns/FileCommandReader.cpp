#include "FileCommandReader.h"
#include <unordered_set>
#include <limits>

geng::serial::FileCommandReader::FileCommandStream::FileCommandStream(FileCommandReader& rOwner,
	const std::shared_ptr<ISerializableCommand>& pCommand)
	:m_rOwner(rOwner),
	m_pCommand(pCommand),
	m_pDeltaHolder(pCommand->AllocateDeltaObject())
{

}

bool geng::serial::FileCommandReader::FileCommandStream::UpdateOnFrame(unsigned long frameIndex)
{
	// Update the owner 
	// Only the first stream updates to the current frame.  Once the frame has been updated,
	// the other streams don't affect it

	if (m_rOwner.CurrentFrame() < frameIndex)
	{
		m_rOwner.SetFrame(frameIndex);
	}

	return true;
}

bool geng::serial::FileCommandReader::FileCommandStream::ReadDelta(FileReadStream& rStream)
{
	// Read from the stream into my delta
	return m_pDeltaHolder->Read(&rStream);
}

bool geng::serial::FileCommandReader::FileCommandStream::ApplyDelta()
{
	m_pDeltaHolder->ApplyTo(*m_pCommand);
	return true;
}

geng::serial::FileCommandReader::FileCommandReader(FileUPtr&& pFile,
	const std::vector<std::shared_ptr<ISerializableCommand> >& commandList)
	:m_fileStream(std::move(pFile))
{
	// Read the header, which is a list of keys
	uint16_t keySize{ 0 };
	constexpr size_t sizeOfSize = sizeof(decltype(keySize));

	decltype(keySize) currentMaxSize{ 0 };

	bool shouldRead = true;

	std::unique_ptr<char[]> pKeyBuff;

	uint32_t keyIndex{ 0 };
	std::string cmdKey;

	while (shouldRead)
	{
		// Try to read the key size
		if (m_fileStream.Read(&keySize, sizeOfSize) < sizeOfSize)
		{
			m_error = "Command file: could not read command key size";
			return;
		}

		if (keySize == 0)
		{
			shouldRead = false;
			continue;
		}

		// Read the key
		if (currentMaxSize < keySize)
		{
			pKeyBuff.reset(new char[keySize]);
			currentMaxSize = keySize;
		}

		if (m_fileStream.Read(pKeyBuff.get(), keySize) < keySize)
		{
			m_error = "Command file: could not read command key";
			return;
		}

		cmdKey.assign(pKeyBuff.get(), keySize);
		m_commandStreamMap.emplace(std::move(cmdKey), keyIndex);
		m_commandStreams.emplace_back();

		if (keyIndex == std::numeric_limits<decltype(keyIndex)>::max())
		{
			// Security
			m_error = "Command file: too many command types";
			return;
		}

		++keyIndex;
	}

	// Try to match the commands with keys in the file
	std::unordered_set<std::string> unmatchedCommands;
	for (const auto& commandKeyPair : m_commandStreamMap)
	{
		unmatchedCommands.emplace(commandKeyPair.first);
	}

	for (const std::shared_ptr<ISerializableCommand>& pCommand
		: commandList)
	{
		std::string commandKey = pCommand->GetKey();

		auto itCommandEntry = m_commandStreamMap.find(commandKey);
		if (itCommandEntry == m_commandStreamMap.end())
		{
			m_error = "Command file: unrecognized command type: ";
			m_error += commandKey;
			return;
		}

		auto idxCommand = itCommandEntry->second;
		m_commandStreams[idxCommand].cmdStream.reset(new FileCommandStream(*this, pCommand));

		unmatchedCommands.erase(commandKey);
	}

	if (!unmatchedCommands.empty())
	{
		m_error = "Command file: command types missing -- are you running an older version?";
		return;
	}

	if (!LoadNextFrame())
	{
		m_fileComplete = true;
	}

	// Done.  
	m_valid = true;
}

const std::shared_ptr<geng::ICommandStream>& geng::serial::FileCommandReader::GetCommandStream(const char* pCommandKey)
{
	static std::shared_ptr<geng::ICommandStream> nullStream;

	auto itStream = m_commandStreamMap.find(std::string(pCommandKey));

	if (itStream != m_commandStreamMap.end())
	{
		return m_commandStreams[itStream->second].cmdStream;
	}

	return nullStream;
}

bool geng::serial::FileCommandReader::LoadNextFrame()
{
	// Read the next frame from the file into the buffer, putting the contents into
	// the deltas

	// 1. Frame number
	uint32_t frameNumber;
	if (m_fileStream.Read(&frameNumber, sizeof(frameNumber)) < sizeof(frameNumber))
	{
		return false;
	}

	if (m_nextFrame != 0 
		&& frameNumber <= m_nextFrame)
	{
		return false;
	}

	m_nextFrame = frameNumber;

	// 2. Number of deltas
	uint32_t deltaCount;
	if (m_fileStream.Read(&deltaCount, sizeof(deltaCount)) < sizeof(deltaCount))
	{
		return false;
	}

	// Read each delta into its command
	m_nextDeltas.clear();

	for (uint32_t iDelta = 0; iDelta < deltaCount; ++iDelta)
	{
		uint32_t commandIndex;
		if (m_fileStream.Read(&commandIndex, sizeof(commandIndex)) < sizeof(commandIndex))
		{
			return false;
		}

		if (!m_commandStreams[commandIndex].cmdStream->ReadDelta(m_fileStream))
		{
			return false;
		}
		m_nextDeltas.emplace_back(commandIndex);
	}

	return true;
}

bool geng::serial::FileCommandReader::SetFrame(unsigned long curFrame)
{
	if (m_nextFrame < curFrame)
	{
		m_currentFrame = curFrame;
		return true;
	}
	else if (m_nextFrame > curFrame)
	{
		return false;
	}

	// Apply and load the next frame
	for (size_t deltaIndex : m_nextDeltas)
	{
		m_commandStreams[deltaIndex].cmdStream->ApplyDelta();
	}

	// This will update the next frame
	if (!LoadNextFrame())
	{
		return false;
	}

	m_currentFrame = curFrame;
	return true;
}

