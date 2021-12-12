#include "ColumnsInput.h"
#include "ColumnsExecutive.h"
#include "SharedValueCommand.h"

#include <unordered_set>
#include <sstream>

geng::columns::ColumnsInput::ColumnsInput(const std::vector<ActionDesc>& vActions, unsigned int playerCount, 
	unsigned long msPerFrame)
	:BaseGameComponent(ColumnsExecutive::GetColumnsInputComponentName()),
	m_actionTranslator(new ActionTranslator()),
	m_seedCommand(new SharedValueCommand<RandomSeedType>("random_seed_shared_value")),
	m_seedValue(new SharedValue<RandomSeedType>()),
	m_msPerFrame(msPerFrame)
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

	return true;
}

void geng::columns::ColumnsInput::OnStartGame(const InputArgs& args)
{
	std::vector<CommandDesc>  commandDescriptions;

	FactorySharedPtr<ICommandStream> pSeedFactory 
		{ CreateFactoryWithArgs<SharedValueCommandStream<RandomSeedType>, 
								ICommandStream>(m_seedCommand, m_seedValue) 
		};
	
	commandDescriptions.emplace_back(m_seedCommand, pSeedFactory);

	// Iterate through all the stored actions, and specify "throttled command streams" for the current player's
	// actions.
	for (const ActionCommand_& actionCommand : m_actionCommands)
	{
		if (actionCommand.playerId == args.userPlayer)
		{
			ActionCommandStreamArgs actionStreamArgs(actionCommand.pCommand,
				m_actionTranslator,
				m_msPerFrame,
				m_actionMapper->GetAction(actionCommand.actionName.c_str()));
			FactorySharedPtr<ICommandStream> pActionFactory
			{ CreateFactoryWithArgs<ThrottledActionCommandStream, ICommandStream>(actionStreamArgs)
			};

			commandDescriptions.emplace_back(actionCommand.pCommand, pActionFactory);
		}
	}

	// Create a new command-manager
	m_pCommandManager.reset(new CommandManager(args.pbMode, args.pFileName, commandDescriptions));

	// Generate the seed for the random generator (when not playing back; when playing back the seed
	// value will land in the command from the file)

	if (args.pbMode != PlaybackMode::Playback)
	{
		std::random_device randomDevice;
		m_seedValue->targetFrame = 0;
		m_seedValue->val = randomDevice();
	}
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

	// Wrap up
	// TODO:  EndFrame should be called from somewhere else if commands can come from the
	// sim
	m_pCommandManager->EndFrame();

	// Seed the random number generator with the value read from the command
	if (m_seedCommand->GetState().hasVal)
	{
		m_generator.seed(m_seedCommand->GetState().val);
	}
}

void geng::columns::ColumnsInput::OnEndGame()
{

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