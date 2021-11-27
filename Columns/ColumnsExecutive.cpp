#include "ColumnsExecutive.h"
#include "CommonSetup.h"

#include "SDLEventPoller.h"
#include "SDLInput.h"
#include "ColumnsSim.h"
#include "SDLTextKeycodes.h"
#include "ColumnsSDLRenderer.h"
#include <SDL.h>
#include <array>

#include "InputBridge.h"

geng::columns::ColumnsExecutive::ColumnsExecutive()
	:BaseGameComponent(GetExecutiveName()),
	m_initialized(false),
	m_pGame()
{
	m_initialized = true;
}

bool geng::columns::ColumnsExecutive::AddToGame(const std::shared_ptr<IGame>& pGame)
{
	m_pGame = pGame;

	// Precreate certain components
	setup::InitializeSDLRendering(pGame.get(), "Columns", 640 + 320, 480 + 240);
	setup::InitializeResourceLoader(pGame.get());
	auto pPoller = std::static_pointer_cast<sdl::EventPoller>
		(setup::InitializeSDLPoller(pGame.get()));

	m_pInput = std::static_pointer_cast<sdl::Input>(setup::InitializeSDLInput(pGame.get()));
	m_pInput->AddCode(SDLK_p); // pause
	m_pInput->AddCode(SDLK_SPACE); // space

	auto pActionMapper = std::static_pointer_cast<ActionMapper>(setup::InitializeActionMapper(pGame.get(), GetActionMapperName()));
	MapActions(*pActionMapper);

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

	// Create the columns sim
	geng::columns::ColumnsSimArgs simArgs;
	simArgs.actionThrottlePeriod = 250;
	simArgs.dropThrottlePeriod = 100;
	simArgs.boardSize.x = 9;
	simArgs.boardSize.y = 24 + 3;  // 3 invisible squares on top
	simArgs.columnSize = 3;
	// NOTE:  This speed may change!
	simArgs.dropMilliseconds = 600;
	//simArgs.dropMilliseconds = 80;
	simArgs.flashMilliseconds = 300;
	simArgs.flashCount = 3;

	m_pSim = std::make_shared<geng::columns::ColumnsSim>(simArgs);
	pGame->AddComponent(m_pSim);

	// Create the columns SDL renderer
	geng::columns::ColumnsRenderArgs renderArgs;
	renderArgs.renderShadow = 4;
	auto pRenderer = std::make_shared<geng::columns::ColumnsSDLRenderer>(renderArgs);
	pGame->AddComponent(pRenderer);

	// Add the three components (input, sim, renderer) to the context as listeners
	if (!pGame->AddListener(ListenerType::Input, m_simContextId, pInputBridge))
	{
		pGame->LogError("Columns: unable to add input bridge as listener");
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

	return true;
}

void geng::columns::ColumnsExecutive::OnFrame(const SimState& rSimState,
	const SimContextState* pContextState)
{

	KeyState ksPause;
	ksPause.keyCode = SDLK_p;
	KeyState ksSpace;
	ksSpace.keyCode = SDLK_SPACE;

	std::array pksArray{ &ksPause, &ksSpace };
	m_pInput->QueryInput(nullptr, nullptr, pksArray.data(), pksArray.size());

	if (m_cheatsEnabled && m_contextDesc == ContextDesc::ActiveGame)
	{
		UpdateCheatState(rSimState.execSimulatedTime);
	}

	// Special key handling
	KeyState resetKey{ 0, 0, KeySignal::KeyUp};
	if (IsInGame())
	{
		// First check if I should switch out of game
		if (m_pSim->IsGameOver())
		{
			m_pGame->SetRunState(m_simContextId, false);
			m_contextDesc = ContextDesc::NoGame;
		}
		else  // Pause handling
		{
			bool pausePressed = IsKeyPressedOnce(ksPause);

			// "p" for pause
			if (pausePressed)
			{
				resetKey.keyCode = SDLK_p;
				m_pInput->ForceState(resetKey);

				if (m_contextDesc == ContextDesc::ActiveGame)
				{
					m_pGame->SetRunState(m_simContextId, false);
					m_contextDesc = ContextDesc::PausedGame;
				}
				else
				{
					m_pGame->SetRunState(m_simContextId, true);
					m_contextDesc = ContextDesc::ActiveGame;
				}
			}
		}
	}
	else
	{
		bool spacePressed = IsKeyPressedOnce(ksSpace);
		if (spacePressed)
		{
			resetKey.keyCode = SDLK_SPACE;
			m_pInput->ForceState(resetKey);

			m_pGame->SetRunState(m_simContextId, true);
			m_pSim->ResetGame();
			m_contextDesc = ContextDesc::ActiveGame;
		}
	}

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
			KeyState kstate{ alphaKeys[i], 0, KeySignal::KeyUp };
			m_alphaKeyStates[i] = kstate;
			m_alphaKeyRefs.emplace_back(&m_alphaKeyStates[i]);
		}
	}

	for (size_t i = 0; i < m_alphaKeyStates.size(); ++i)
	{
		pInput->AddCode(m_alphaKeyStates[i].keyCode);
	}
}

void geng::columns::ColumnsExecutive::MapActions(ActionMapper& rMapper)
{
	auto dropAction = rMapper.CreateAction(GetDropActionName());
	rMapper.MapAction(dropAction, SDLK_s);
	rMapper.MapAction(dropAction, SDLK_DOWN);

	auto leftAction = rMapper.CreateAction(GetShiftLeftActionName());
	rMapper.MapAction(leftAction, SDLK_a);
	rMapper.MapAction(leftAction, SDLK_LEFT);

	auto rightAction = rMapper.CreateAction(GetShiftRightActionName());
	rMapper.MapAction(rightAction, SDLK_d);
	rMapper.MapAction(rightAction, SDLK_RIGHT);

	auto rotateAction = rMapper.CreateAction(GetRotateActionName());
	rMapper.MapAction(rotateAction, SDLK_SPACE);

	auto permuteAction = rMapper.CreateAction(GetPermuteActionName());
	rMapper.MapAction(permuteAction, SDLK_r);

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
const char* geng::columns::ColumnsExecutive::GetExecutiveName()
{
	return "ColumnsExecutive";
}