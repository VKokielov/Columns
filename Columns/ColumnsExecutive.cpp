#include "ColumnsExecutive.h"
#include "CommonSetup.h"


#include "SDLEventPoller.h"
#include "SDLInput.h"
#include "ColumnsSim.h"
#include "SDLTextKeycodes.h"
#include "ColumnsSDLRenderer.h"
#include "ColumnsInput.h"
#include <SDL.h>
#include <array>

#include "InputBridge.h"

geng::columns::ColumnsExecutive::ColumnsExecutive(const ExecutiveSettings& settings)
	:BaseGameComponent(GetExecutiveName()),
	m_initialized(false),
	m_pGame()
{
	m_initialized = true;
	m_inputArgs.pbMode = settings.pbMode;
	m_inputArgs.fileName = settings.pbFileName;
	m_inputArgs.userPlayer = 0;
}

bool geng::columns::ColumnsExecutive::AddToGame(const std::shared_ptr<IGame>& pGame)
{
	// Precreate certain components
	setup::InitializeSDLRendering(pGame.get(), "Columns", 640 + 320, 480 + 240);
	setup::InitializeResourceLoader(pGame.get());
	auto pPoller = std::static_pointer_cast<sdl::EventPoller>
		(setup::InitializeSDLPoller(pGame.get()));

	m_pInput = std::static_pointer_cast<sdl::Input>(setup::InitializeSDLInput(pGame.get()));
	m_spaceKey.keyCode = SDLK_SPACE;
	AddKeySub(&m_spaceKey);

	m_pauseKey.keyCode = SDLK_p;
	AddKeySub(&m_pauseKey);

	m_escKey.keyCode = SDLK_ESCAPE;
	AddKeySub(&m_escKey);
	
	ThrottleSettings throttleSettings;
	throttleSettings.dropThrottlePeriod = 100;
	throttleSettings.nonDropThrottlePeriod = 250;

	auto pActionMapper = std::static_pointer_cast<ActionMapper>(setup::InitializeActionMapper(pGame.get(), GetActionMapperName()));
	std::vector<ActionDesc>  actionDescriptions;
	MapActions(*pActionMapper, actionDescriptions, throttleSettings);

	// Add the event poller and the overall input as executive listeners to execute before the executive itself
	// This means they will run first in any frame, regardless of context
	if (!pGame->AddListener(ListenerType::Executive, EXECUTIVE_CONTEXT,
		pPoller))
	{
		pGame->LogError("Columns: unable to add SDL event poller as listener");
		return false;
	}

	if (!pGame->AddListener(ListenerType::Executive, EXECUTIVE_CONTEXT,
		m_pInput))
	{
		pGame->LogError("Columns: unable to add SDL input handler as listener");
		return false;
	}

	if (!pGame->AddListener(ListenerType::Executive, EXECUTIVE_CONTEXT,
		shared_from_this()))
	{
		pGame->LogError("Columns: unable to add executive as listener");
		return false;
	}

	// Create the columns sim context
	m_simContextId = pGame->CreateSimContext(GetColumnsSimContextName());

	// Create the columns input bridge
	auto pInputBridge = std::make_shared<InputBridge>(GetColumnsInputBridgeName(), m_pInput);
	pGame->AddComponent(pInputBridge);

	// Create the columns input component
	m_pColumnsInput = std::make_shared<ColumnsInput>(actionDescriptions, 
														2,  // note: MAXIMUM player count! 
														pGame->GetGameArgs().msTimePerFrame);
	pGame->AddComponent(m_pColumnsInput);

	// Preinitialize, but do not yet use, the columns sim arguments

	m_simArgs.boardSize.x = 9;
	m_simArgs.boardSize.y = 24 + 3;  // 3 invisible squares on top
	m_simArgs.columnSize = 3;
	// NOTE:  This speed may change!
	m_simArgs.dropMilliseconds = 600;
	m_simArgs.flashMilliseconds = 300;
	m_simArgs.flashCount = 3;

	m_pSim = std::make_shared<geng::columns::ColumnsSim>();
	pGame->AddComponent(m_pSim);

	// Create the columns SDL renderer
	geng::columns::ColumnsRenderArgs renderArgs;
	renderArgs.renderShadow = 4;
	auto pRenderer = std::make_shared<geng::columns::ColumnsSDLRenderer>(renderArgs);
	pGame->AddComponent(pRenderer);
	m_pSDLRenderer = pRenderer;
	
	// Add the three components (input, sim, renderer) to the context as listeners
	if (!pGame->AddListener(ListenerType::Input, m_simContextId, pInputBridge))
	{
		pGame->LogError("Columns: unable to add input bridge as listener");
		return false;
	}

	if (!pGame->AddListener(ListenerType::Input, m_simContextId, m_pColumnsInput))
	{
		pGame->LogError("Columns: unable to add Colunns input component as listener");
		return false;
	}

	if (!pGame->AddListener(ListenerType::Simulation, m_simContextId, m_pSim))
	{
		pGame->LogError("Columns: unable to add simulation as listener");
		return false;
	}

	if (!pGame->AddListener(ListenerType::Rendering, m_simContextId, pRenderer))
	{
		pGame->LogError("Columns: unable to add renderer as listener");
		return false;
	}

	// Set focus, visibility, runstate...
	pGame->SetFocus(m_simContextId);
	pGame->SetVisibility(m_simContextId, true);
	pGame->SetRunState(m_simContextId, false);

	SetupCheats(m_pInput.get());

	m_pGame = pGame;

	return true;
}

void geng::columns::ColumnsExecutive::StartGame()
{
	if (m_contextState == ContextState::NoGame)
	{
		Transition<ActiveGameState>(*this);
	}
}
void geng::columns::ColumnsExecutive::EndGame()
{
	if (IsInGameState(m_contextState))
	{
		Transition<NoGameState>(*this);
	}
}
void geng::columns::ColumnsExecutive::PauseGame(bool pauseState)
{
	if (pauseState)
	{
		if (m_contextState == ContextState::ActiveGame)
		{
			Transition<PausedGameState>(*this);
		}
	}
	else
	{
		if (m_contextState == ContextState::PausedGame)
		{
			Transition<ActiveGameState>(*this);
		}
	}
}

// Overall state
void geng::columns::ColumnsExecutive::OnEnterState(NoGameState& ngs)
{
	m_prevContextState = m_contextState;
	m_contextState = ContextState::NoGame;

	if (IsInGameState(m_prevContextState))
	{
		m_pSDLRenderer->OnEndGame();
		m_pSim->OnEndGame();
		m_pColumnsInput->OnEndGame();

		// Suspend execution
		auto pGame = m_pGame.lock();

		if (pGame)
		{
			pGame->SetRunState(m_simContextId, false);
		}
	}
}

void geng::columns::ColumnsExecutive::OnExitState(NoGameState& ngs) {}
void geng::columns::ColumnsExecutive::OnEnterState(ActiveGameState& ags)
{
	m_prevContextState = m_contextState;
	m_contextState = ContextState::ActiveGame;

	if (!IsInGameState(m_prevContextState))
	{
		// Reset the counter
		auto pGame = m_pGame.lock();

		if (pGame)
		{
			pGame->SetFrameIndex(m_simContextId, 0);
		}

		m_pColumnsInput->OnStartGame(m_inputArgs);
		m_pSim->OnStartGame(m_simArgs);
		m_pSDLRenderer->OnStartGame();

		// Begin execution
		if (pGame)
		{
			pGame->SetRunState(m_simContextId, true);
		}
	}
	else if (IsPausedState(m_prevContextState))
	{
		// Suspend execution
		auto pGame = m_pGame.lock();
		if (pGame)
		{
			pGame->SetRunState(m_simContextId, true);
		}

		m_pColumnsInput->OnPauseGame(false);
		m_pSim->OnPauseGame(false);
		m_pSDLRenderer->OnPauseGame(false);
	}
	
}
void geng::columns::ColumnsExecutive::OnExitState(ActiveGameState& ags) {}
void geng::columns::ColumnsExecutive::OnEnterState(PausedGameState& pgs)
{
	m_prevContextState = m_contextState;
	m_contextState = ContextState::PausedGame;

	// Suspend execution
	auto pGame = m_pGame.lock();
	if (pGame)
	{
		pGame->SetRunState(m_simContextId, false);
	}

	// Notify
	m_pColumnsInput->OnPauseGame(true);
	m_pSim->OnPauseGame(true);
	m_pSDLRenderer->OnPauseGame(true);
}

void geng::columns::ColumnsExecutive::OnExitState(PausedGameState& pgs) {}

void geng::columns::ColumnsExecutive::OnFrame(NoGameState&, const SimState& rSimState)
{
	if (IsKeyPressedOnce(m_spaceKey))
	{
		ResetKey(&m_spaceKey);
		StartGame();
	}

}
void geng::columns::ColumnsExecutive::OnFrame(ActiveGameState&, const SimState& rState)
{
	if (m_cheatsEnabled)
	{
		UpdateCheatState(rState.execSimulatedTime);
	}

	// Pause handling
	if (IsKeyPressedOnce(m_pauseKey))
	{
		ResetKey(&m_pauseKey);
		PauseGame(true);
	}

	if (IsKeyPressedOnce(m_escKey))
	{
		// End
		ResetKey(&m_escKey);
		EndGame();
	}

}
void geng::columns::ColumnsExecutive::OnFrame(PausedGameState&, const SimState& rState)
{

	// Pause handling
	if (IsKeyPressedOnce(m_pauseKey))
	{
		ResetKey(&m_pauseKey);
		PauseGame(false);
	}
}

void geng::columns::ColumnsExecutive::AddKeySub(KeyState* pkeyState)
{
	// Add each key once.  Adding one twice is not harmful but a waste of time
	m_pInput->AddCode(pkeyState->keyCode);
	m_keySubs.push_back(pkeyState);
}
void geng::columns::ColumnsExecutive::ResetKey(KeyState* pKeyState)
{
	KeyState resetKey{ pKeyState->keyCode};
	resetKey.finalState = KeySignal::KeyUp;
	resetKey.numChanges = 0;
	m_pInput->ForceState(resetKey);
}

void geng::columns::ColumnsExecutive::OnFrame(const SimState& rSimState,
	const SimContextState* pContextState)
{
	// Key states
	m_pInput->QueryInput(nullptr, nullptr, m_keySubs.data(), m_keySubs.size());

	InvokeHelper<OnFrameSelector, ColumnsExecutive>
		invokeHelperOnFrame(*this);

	DispatchInvoke(invokeHelperOnFrame, rSimState);
}

void geng::columns::ColumnsExecutive::UpdateCheatState(unsigned long execTime)
{
	if (m_cheatKey.has_value())
	{
		// Do not react to inputs until output has been read off and reset
		return;
	}

	if (m_cheatState != CHEAT_TRIE_ROOT
		&& (m_lastCheatInputTime + msToEnterCheat <= execTime))
	{
		// Cheat is invalidated
		m_cheatState = CHEAT_TRIE_ROOT;
	}

	m_pInput->QueryInput(nullptr, nullptr, m_alphaKeyRefs.data(), m_alphaKeyRefs.size());

	// Find the first active key
	char curC;
	bool hasChar{ false };
	for (KeyState& state : m_alphaKeyStates)
	{
		if (IsKeyPressedOnce(state))
		{
			hasChar = true;
			curC = sdl::GetTextFromKeyCode(state.keyCode);
			break;
		}
	}

	if (hasChar)
	{
//		fprintf(stderr, "got char %c\n", curC);
		TrieIndex nextNode = m_cheatTrie.GetNext(m_cheatState, curC);
		if (nextNode != CHEAT_TRIE_ROOT)
		{
			// Advance
			m_lastCheatInputTime = execTime;
			m_cheatState = nextNode;
			// Cheat?
			if (m_cheatTrie.HasKey(m_cheatState))
			{
//				fprintf(stderr, "Has key!!\n");
				m_cheatKey.emplace(m_cheatTrie.GetKey(m_cheatState));
				m_cheatState = CHEAT_TRIE_ROOT;
			}
		}
	}
}

void geng::columns::ColumnsExecutive::SetupCheats(IInput* pInput)
{
	// This may be called multiple times to do identical work but how often?
	// Leave it this way for simplicity

	if (m_alphaKeyStates.empty())
	{
		std::vector<KeyCode> alphaKeys;
		sdl::AddAllTextKeys(alphaKeys);

		// Resize the key state array to prevent reallocations
		m_alphaKeyStates.resize(alphaKeys.size());
		for (size_t i = 0; i < alphaKeys.size(); ++i)
		{
			m_alphaKeyStates[i].keyCode = alphaKeys[i];
			m_alphaKeyStates[i].numChanges = 0;
			m_alphaKeyStates[i].finalState = KeySignal::KeyUp;
			m_alphaKeyRefs.emplace_back(&m_alphaKeyStates[i]);
		}
	}

	for (size_t i = 0; i < m_alphaKeyStates.size(); ++i)
	{
		pInput->AddCode(m_alphaKeyStates[i].keyCode);
	}
}

void geng::columns::ColumnsExecutive::MapActions(ActionMapper& rMapper, 
	std::vector<ActionDesc>& columnsActions,
	const ThrottleSettings& throttleSettings)
{
	auto dropAction = rMapper.CreateAction(GetDropActionName());
	rMapper.MapAction(dropAction, SDLK_s);
	rMapper.MapAction(dropAction, SDLK_DOWN);
	columnsActions.emplace_back(GetDropActionName(), throttleSettings.dropThrottlePeriod);

	auto leftAction = rMapper.CreateAction(GetShiftLeftActionName());
	rMapper.MapAction(leftAction, SDLK_a);
	rMapper.MapAction(leftAction, SDLK_LEFT);
	columnsActions.emplace_back(GetShiftLeftActionName(), throttleSettings.nonDropThrottlePeriod);

	auto rightAction = rMapper.CreateAction(GetShiftRightActionName());
	rMapper.MapAction(rightAction, SDLK_d);
	rMapper.MapAction(rightAction, SDLK_RIGHT);
	columnsActions.emplace_back(GetShiftRightActionName(), throttleSettings.nonDropThrottlePeriod);

	auto rotateAction = rMapper.CreateAction(GetRotateActionName());
	rMapper.MapAction(rotateAction, SDLK_SPACE);
	columnsActions.emplace_back(GetRotateActionName(), throttleSettings.nonDropThrottlePeriod);

	auto permuteAction = rMapper.CreateAction(GetPermuteActionName());
	rMapper.MapAction(permuteAction, SDLK_r);
	columnsActions.emplace_back(GetPermuteActionName(), throttleSettings.nonDropThrottlePeriod);
}

void geng::columns::ColumnsExecutive::AddCheat(const char* pText, CheatKey key)
{
	m_cheatTrie.AddEntry(pText, key);
}

const char* geng::columns::ColumnsExecutive::GetDropActionName()
{
	return "DropColumnAction";
}
const char* geng::columns::ColumnsExecutive::GetShiftLeftActionName()
{
	return "ShiftColumnLeftAction";
}
const char* geng::columns::ColumnsExecutive::GetShiftRightActionName()
{
	return "ShiftColumnRightAction";
}
const char* geng::columns::ColumnsExecutive::GetRotateActionName()
{
	return "RotateColumnAction";
}
const char* geng::columns::ColumnsExecutive::GetPermuteActionName()
{
	return "PermuteColumnAction";
}
const char* geng::columns::ColumnsExecutive::GetColumnsSimContextName()
{
	return "ColumnsSimContext";
}
const char* geng::columns::ColumnsExecutive::GetActionMapperName()
{
	return "ActionMapper";
}
const char* geng::columns::ColumnsExecutive::GetColumnsInputBridgeName()
{
	return "ColumnsInputBridge";
}
const char* geng::columns::ColumnsExecutive::GetColumnsInputComponentName()
{
	return "ColumnsInputComponent";
}
const char* geng::columns::ColumnsExecutive::GetExecutiveName()
{
	return "ColumnsExecutive";
}