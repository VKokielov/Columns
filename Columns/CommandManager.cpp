#include "CommandManager.h"
#include "FileCommandReader.h"
#include "FileCommandWriter.h"

geng::CommandManager::CommandManager(PlaybackMode pbMode,
	const char* pPBFile,
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
	if (!OpenFile(commandList))
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

bool geng::CommandManager::OpenFile(const std::vector<std::shared_ptr<serial::ISerializableCommand> >& commandList)
{
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
								(std::move(filePtr), commandList);

			if (!m_pReader->IsValid())
			{
				m_error = m_pReader->GetError();
				return false;
			}
		}
		else
		{
			m_pWriter = std::make_shared<serial::FileCommandWriter>
								(std::move(filePtr), commandList);
			if (!m_pWriter->IsValid())
			{
				m_error = m_pWriter->GetError();
				return false;
			}
		}
	}

	return true;
}