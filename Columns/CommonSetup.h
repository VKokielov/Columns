#pragma once

#include "IGame.h"
#include <memory>

namespace geng::setup
{
	std::shared_ptr<IGameComponent> InitializeResourceLoader(geng::IGame* pGame);
	std::shared_ptr<IGameComponent> InitializeSDLPoller(geng::IGame* pGame);
	std::shared_ptr<IGameComponent> InitializeSDLInput(geng::IGame* pGame);
	std::shared_ptr<IGameComponent> InitializeActionMapper(geng::IGame* pGame, 
															const char* pName);

	std::shared_ptr<IGameComponent> InitializeSDLRendering(geng::IGame* pGame,
														  const char* pWindowName,
														  int windowX,
														  int windowY);

}