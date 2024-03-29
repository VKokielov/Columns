#include "ColumnsInput.h"
#include "ColumnsExecutive.h"
#include "SharedValueCommand.h"

#include <unordered_set>
#include <sstream>

geng::columns::ColumnsInput::ColumnsInput(const std::vector<ActionDesc>& vActions, unsigned int playerCount,
	unsigned long msPerFrame)
	:BaseGameComponent(ColumnsExecutive::GetColumnsInputComponentName()),
	m_actionTranslator(new ActionTranslator()),
	m_msPerFrame(msPerFrame),
	m_pSimArgsPacket(new serial::DataPacket<SimArgs>())
{
	std::unordered_set<std::string>  actionNames;

	for (const ActionDesc& adesc : vActions)
	{
		std::string actionName(adesc.pName);
		if (actionNames.count(actionName))
		{
			m_error = "ColumnsInput: Duplicate action name: ";
			m_error += adesc.pName;
			return;
		}

		actionNames.emplace(actionName);
		m_actionNames.emplace_back(actionName);
	}

	for (unsigned int playerId = 0; playerId < playerCount; ++playerId)
	{
		for (const ActionDesc& adesc : vActions)
		{
			std::stringstream actionKeyStream;
			actionKeyStream << "p" << playerId << "_" << adesc.pName;
			std::string actionKey = actionKeyStream.str();

			auto pCommand = std::make_shared<ActionCommand>(actionKey.c_str());
			m_actionCommands.emplace_back(adesc.pName, actionKey, playerId, pCommand, adesc.throttlePeriod);
		}
	}

	m_valid = true;
}

bool geng::columns::ColumnsInput::Initialize(const std::shared_ptr<IGame>& pGame)
{
	if (!m_valid)
	{
		pGame->LogError(m_error.c_str());
		return false;
	}
	
	GetComponentResult getResult;
	m_pInput = GetComponentAs<IInput>(pGame.get(), ColumnsExecutive::GetColumnsInputBridgeName(), getResult);

	if (!m_pInput)
	{
		pGame->LogError("ColumnsInput: could not get input component");
		return false;
	}

	m_actionMapper = GetComponentAs<ActionMapper>(pGame.get(), ColumnsExecutive::GetActionMapperName(), getResult);

	if (!m_actionMapper)
	{
		pGame->LogError("ColumnsInput: could not get action mapper");
		return false;
	}

	if (!m_actionTranslator->InitActions(m_actionMapper.get(),
		m_actionNames.begin(),
		m_actionNames.end()))
	{
		pGame->LogError("ColumnsInput: could not initialize action translator -- check action names.");
		return false;
	}

	m_actionTranslator->SetInput(m_pInput);
	m_actionMapper->GetAllMappings(m_actionTranslator);
	m_actionMapper->AddMappingListener(m_actionTranslator);

	auto pExecutive = GetComponentAs<ColumnsExecutive>(pGame.get(), ColumnsExecutive::GetExecutiveName());
	m_pExecutive = pExecutive;

	return true;
}

void geng::columns::ColumnsInput::OnStartGame(const ColumnsArgs& args)
{
	std::vector<CommandDesc>  commandDescriptions;

	// Iterate through all the stored actions, and specify "throttled command streams" for the current player's
	// actions.
	for (const ActionCommand_& actionCommand : m_actionCommands)
	{
		if (actionCommand.playerId == args.inputArgs.userPlayer)
		{
			ActionCommandStreamArgs actionStreamArgs(actionCommand.pCommand,
				m_actionTranslator,
				m_msPerFrame,
				m_actionMapper->GetAction(actionCommand.actionName.c_str()));
			FactorySharedPtr<ICommandStream> pActionFactory
			{ CreateFactoryWithArgs<ThrottledActionCommandStream, ICommandStream>(actionStreamArgs, actionCommand.throttlePeriod)
			};

			commandDescriptions.emplace_back(actionCommand.pCommand, pActionFactory);
		}
	}


	const InputArgs& inputArgs = args.inputArgs;
	const SimArgs& simArgs = args.simArgs;

	// Create a new command-manager with a description packet
	if (inputArgs.pbMode != PlaybackMode::Playback)
	{
		// Assign the sim args as given to the lvalue "data packet" which will potentially
		// be passed along for recording
		m_pSimArgsPacket->Get() = simArgs;
		std::random_device randomDevice;
		m_pSimArgsPacket->Get().randomSeed = randomDevice();
	}

	m_pCommandManager.reset(new CommandManager(inputArgs.pbMode, 
										      inputArgs.fileName.c_str(),
											  ColumnsExecutive::GetGameName(),
		                                      0,   // format version
		                                      0,   // min format version,
										      false, // unsafe playback
											  m_pSimArgsPacket,
										      commandDescriptions));

	if (!m_pCommandManager->IsValid())
	{
		auto pExecutive = m_pExecutive.lock();
		if (pExecutive)
		{
			pExecutive->StartGameError(m_pCommandManager->GetError().c_str());
			return;
		}
	}

	// Seed the random generator (the sim args will have a valid value now)
	m_generator.seed(m_pSimArgsPacket->Get().randomSeed);
}

void geng::columns::ColumnsInput::OnFrame(const SimState& rSimState,
	const SimContextState* pContextState)
{
	if (!pContextState->runstate.curValue)
	{
		// Nothing to do here if we're not running
		return;
	}

	// This will update the translator with the state of the input
	m_actionTranslator->UpdateOnFrame(pContextState->frameCount);

	// Calling Update on the command manager will pull in the values from each stream
	m_pCommandManager->OnFrame(pContextState->frameCount);

	// TODO:  EndFrame should be called from somewhere else if commands can come from the
	// sim
	m_pCommandManager->EndFrame();

	// End the game if in playback mode and the file is done
	if (m_pCommandManager->IsEndOfPlayback())
	{
		auto pExecutive = m_pExecutive.lock();
		if (pExecutive)
		{
			pExecutive->EndGame();
		}
	}
}

void geng::columns::ColumnsInput::OnEndGame()
{
	m_pCommandManager->EndSession();
}

geng::columns::ActionCommandID geng::columns::ColumnsInput::GetIDFor(const char* pActionName,
	unsigned int playerId,
	bool* pFound) const
{
	for (ActionCommandID cmdId = 0; cmdId < m_actionCommands.size(); ++cmdId)
	{
		if (m_actionCommands[cmdId].playerId == playerId
			&& m_actionCommands[cmdId].actionName.compare(pActionName) == 0)
		{
			if (pFound)
			{
				*pFound = true;
			}

			return cmdId;
		}
	}

	if (pFound)
	{
		*pFound = false;
	}

	return m_actionCommands.size();
}

unsigned long geng::columns::ColumnsInput::GetRandomNumber(unsigned long lowerBound, unsigned long upperBound)
{
	return m_generator() % (upperBound + 1 - lowerBound) + lowerBound;
}