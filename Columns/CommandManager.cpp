#include "CommandManager.h"
#include "FileCommandReader.h"
#include "FileCommandWriter.h"

#include <sstream>

geng::CommandManager::CommandManager(PlaybackMode pbMode,
	const char* pPBFile,
	const char* pGameName,
	uint32_t formatVersion,
	uint32_t minVersion,
	bool allowUnsafePlayback,
	const std::shared_ptr<serial::IPacket>& pDescriptionPacket,
	const std::vector<CommandDesc>& cmdList)
	:m_playbackMode(pbMode),
	m_fileName(pPBFile)
{
	// Use the command list to construct a vector of serializable commands
	std::vector<std::shared_ptr<serial::ISerializableCommand> > commandList;

	for (const CommandDesc& cmdDesc : cmdList)
	{
		if (!cmdDesc.m_pCommand)
		{
			// Skip nullptr (bug)
			continue;
		}

		Command_ cmdInManager;
		cmdInManager.pCommand = cmdDesc.m_pCommand;
		commandList.emplace_back(cmdInManager.pCommand);

		if (UserControl(pbMode))
		{
			// Create the stream too when it's used here
			cmdInManager.pStream.reset(cmdDesc.m_pStreamFactory->Create());

			if (!cmdInManager.pStream)
			{
				m_error = "CommandManager: could not create user command stream: ";
				m_error += cmdDesc.m_pCommand->GetKey();
				return;
			}
		}

		m_commands.emplace_back(std::move(cmdInManager));
	}

	// Open the file if applicable.  For reading, create the playback streams
	if (!OpenFile(pGameName, 
		formatVersion, 
		minVersion, 
		allowUnsafePlayback, 
		pDescriptionPacket, 
		commandList))
	{
		return;
	}

	// Now, if the mode is "playback", create the streams from the reader and bind them
	if (pbMode == PlaybackMode::Playback)
	{
		for (Command_& cmdInManager : m_commands)
		{
			cmdInManager.pStream = m_pReader->GetCommandStream(cmdInManager.pCommand->GetKey());
		}
	}
	// And if the mode is "record", subscribe the writer as a listener to each of its commands
	// Uses the special "SubscribeToCommands" function, which takes a shared_ptr instead of a
	// normal this pointer
	else if (pbMode == PlaybackMode::Record)
	{
		serial::FileCommandWriter::SubscribeToCommands(m_pWriter, true);
	}

	m_valid = true;
}

geng::CommandManager::~CommandManager()
{
	if (m_playbackMode == PlaybackMode::Record)
	{
		// This removes the circular references from the commands back to the writer
		serial::FileCommandWriter::SubscribeToCommands(m_pWriter, false);
	}
}

bool geng::CommandManager::IsEndOfPlayback() const
{
	return m_playbackMode == PlaybackMode::Playback
		&& m_pReader->IsWrappedUp();
}

void geng::CommandManager::OnFrame(unsigned long frameIndex)
{
	if (m_playbackMode == PlaybackMode::Record)
	{
		m_pWriter->BeginFrame(frameIndex);
	}
	
	// NOTE:  For playback, the first command stream called invokes the reader's 
	// "update frame" function; so it need not be done explicitly.
	for (Command_& cmdInManager : m_commands)
	{
		cmdInManager.pStream->UpdateOnFrame(frameIndex);
	}
}

void geng::CommandManager::EndFrame()
{
	if (m_playbackMode == PlaybackMode::Record)
	{
		m_pWriter->EndFrame();
	}
}

void geng::CommandManager::EndSession()
{
	if (m_playbackMode == PlaybackMode::Record)
	{
		m_pWriter->EndSession();
	}
	m_playbackMode = PlaybackMode::Ended;
}

bool geng::CommandManager::OpenFile(
	const char* pGameName,
	uint32_t formatVersion,
	uint32_t minVersion,
	bool allowUnsafePlayback,
	const std::shared_ptr<serial::IPacket>& pDescriptionPacket,
	const std::vector<std::shared_ptr<serial::ISerializableCommand> >& commandList)
{
	serial::FileStreamHeader hdrDesc;
	
	hdrDesc.headerConstant = "DemoFile_";
	hdrDesc.headerConstant += pGameName;
	hdrDesc.versionNo = formatVersion;
	hdrDesc.hasChecksum = true;
	hdrDesc.checksumSeed = SEED_DEMO_CHECKSUM;

	if (HasFile(m_playbackMode))
	{
		// Open the file
		const char* fileMode = m_playbackMode == PlaybackMode::Playback ?
			"rb" : "wb";
		FileUPtr filePtr{ fopen(m_fileName.c_str(), fileMode) };

		if (!filePtr)
		{
			if (m_playbackMode == PlaybackMode::Playback)
			{
				m_error = "Could not open playback file for reading";
			}
			else
			{
				m_error = "Could not open playback file for writing";
			}
			return false;
		}

		if (m_playbackMode == PlaybackMode::Playback)
		{
			m_pReader = std::make_shared<serial::FileCommandReader>
								(std::move(filePtr), pDescriptionPacket, commandList, &hdrDesc);
			
			// Check the checksum status before anything else
			if (m_pReader->GetChecksumStatus() != serial::FileChecksumStatus::FileChecksumOK)
			{
				if (!allowUnsafePlayback)
				{
					m_error = "Cannot replay demo with no checksum/invalid checksum";
					return false;
				}

				m_unsafePlayback = true;
			}

			if (!m_pReader->IsValid())
			{
				m_error = m_pReader->GetError();
				return false;
			}

			// Check the version
			if (m_pReader->GetFormatVersion() < minVersion)
			{
				std::stringstream ssm;
				ssm << "Minimum format version required for playback is " << minVersion << "; file version is "
					<< m_pReader->GetFormatVersion();
				m_error = ssm.str();
				return false;
			}

			m_playbackFormatVersion = m_pReader->GetFormatVersion();
		}
		else
		{
			m_pWriter = std::make_shared<serial::FileCommandWriter>
								(std::move(filePtr), pDescriptionPacket, commandList, &hdrDesc);
			if (!m_pWriter->IsValid())
			{
				m_error = m_pWriter->GetError();
				return false;
			}
		}
	}

	return true;
}