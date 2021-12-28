#include "IGame.h"
#include "ResourceLoader.h"
#include "RawMemoryResource.h"
#include "TrueTypeFont.h"
#include "SDLEventPoller.h"
#include "SDLInput.h"
#include "ActionMapper.h"
#include "SDLRendering.h"

#include "CommonSetup.h"

std::shared_ptr<geng::IGameComponent> geng::setup::InitializeResourceLoader(geng::IGame* pGame)
{
	constexpr unsigned int RAW_MEM_BUFFER_SIZE = 1024;

	auto pResLoader = std::make_shared<geng::ResourceLoader>();

	auto pRawFactory = std::make_shared<geng::RawMemoryFactory>(RAW_MEM_BUFFER_SIZE);
	pResLoader->AddFactory(pRawFactory);

	pGame->AddComponent(pResLoader);
	return pResLoader;
}

std::shared_ptr<geng::IGameComponent> geng::setup::InitializeSDLPoller(geng::IGame* pGame)
{
	auto pEventPoller = std::make_shared<geng::sdl::EventPoller>();
	pGame->AddComponent(pEventPoller);
	return pEventPoller;
}

std::shared_ptr<geng::IGameComponent> geng::setup::InitializeSDLInput(geng::IGame* pGame)
{
	auto pInput = std::make_shared<geng::sdl::Input>();
	pGame->AddComponent(pInput);
	return pInput;
}

std::shared_ptr<geng::IGameComponent> geng::setup::InitializeActionMapper(geng::IGame* pGame,
	const char* pName)
{
	// ACTION MAPPER
	auto pActionMapper = std::make_shared<geng::ActionMapper>(pName);
	pGame->AddComponent(pActionMapper);
	return pActionMapper;
}

std::shared_ptr<geng::IGameComponent> geng::setup::InitializeSDLRendering(geng::IGame* pGame,
	const char* pWindowName,
	int windowX,
	int windowY)
{
	auto pRendering = std::make_shared<sdl::SDLRendering>(pWindowName, windowX, windowY);
	pGame->AddComponent(pRendering);

	return pRendering;
}
