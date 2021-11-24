#include "ColumnsExecutive.h"
#include "CommonSetup.h"

#include "SDLEventPoller.h"
#include "SDLInput.h"

geng::columns::ColumnsExecutive::ColumnsExecutive(const std::shared_ptr<IGame>& pGame)
	:BaseGameComponent("ColumnsSimExecutive"),
	m_initialized(false)
{
	// Precreate certain components
	setup::InitializeResourceLoader(pGame.get());
	auto pPoller = std::static_pointer_cast<sdl::EventPoller>
		(setup::InitializeSDLPoller(pGame.get()));

	auto pInput = std::static_pointer_cast<sdl::Input>(setup::InitializeSDLInput(pGame.get()));

	auto pActionMapper = setup::InitializeActionMapper(pGame.get(), GetActionMapperName());

	// Add the event poller and the overall input as executive listeners to execute before the executive itself
	// This means they will run first in any frame, regardless of context
	if (!pGame->AddListener(ListenerType::Executive, EXECUTIVE_CONTEXT,
		pPoller))
	{
		pGame->LogError("Columns: unable to add SDL event poller as listener");
		return;
	}

	if (!pGame->AddListener(ListenerType::Executive, EXECUTIVE_CONTEXT,
		pInput))
	{
		pGame->LogError("Columns: unable to add SDL input handler as listener");
		return;
	}
	
	if (!pGame->AddListener(ListenerType::Executive, EXECUTIVE_CONTEXT,
		shared_from_this()))
	{
		pGame->LogError("Columns: unable to add executive as listener");
		return;
	}

	// Create the columns sim context
	ContextID contextId = pGame->CreateSimContext(GetColumnsSimContextName());

	m_initialized = true;
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