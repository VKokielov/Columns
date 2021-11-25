#include "ColumnsExecutive.h"
#include "CommonSetup.h"

#include "SDLEventPoller.h"
#include "SDLInput.h"
#include "ColumnsSim.h"
#include "ColumnsSDLRenderer.h"
#include <SDL.h>

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
	simArgs.dropThrottlePeriod = 150;
	simArgs.boardSize.x = 9;
	simArgs.boardSize.y = 24 + 3;  // 3 invisible squares on top
	simArgs.columnSize = 3;
	// NOTE:  This speed may change!
	simArgs.dropMilliseconds = 600;
	simArgs.flashMilliseconds = 300;
	simArgs.flashCount = 3;

	auto pSim = std::make_shared<geng::columns::ColumnsSim>(simArgs);
	pGame->AddComponent(pSim);

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

	if (!pGame->AddListener(ListenerType::Simulation, m_simContextId, pSim))
	{
		pGame->LogError("Columns: unable to add simulation as listener");
		return false;
	}

	if (!pGame->AddListener(ListenerType::Rendering, m_simContextId, pSim))
	{
		pGame->LogError("Columns: unable to add renderer as listener");
		return false;
	}

	// Set focus, visibility, runstate...
	pGame->SetFocus(m_simContextId);
	pGame->SetVisibility(m_simContextId, true);
	pGame->SetRunState(m_simContextId, true);

	return true;
}

void geng::columns::ColumnsExecutive::OnFrame(const SimState& rSimState,
	const SimContextState* pContextState)
{
	KeyState ksEsc;
	ksEsc.keyCode = SDLK_p;
	KeyState* pksEsc{ &ksEsc };

	m_pInput->QueryInput(nullptr, nullptr, &pksEsc, 1);

	// "p" for pause
	if (ksEsc.finalState == KeySignal::KeyDown
		|| ksEsc.numChanges > 1)
	{
		if (m_contextDesc == ContextDesc::ActiveGame)
		{
			m_pGame->SetRunState(m_simContextId, false);
			m_paused = true;
		}
		else
		{
			m_pGame->SetRunState(m_simContextId, true);
			m_paused = false;
		}
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
	rMapper.MapAction(leftAction, SDLK_RIGHT);

	auto rotateAction = rMapper.CreateAction(GetRotateActionName());
	rMapper.MapAction(rotateAction, SDLK_SPACE);

	auto permuteAction = rMapper.CreateAction(GetPermuteActionName());
	rMapper.MapAction(permuteAction, SDLK_r);
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