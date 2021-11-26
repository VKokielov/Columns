#include "SDLRendering.h"
#include "SDLHelpers.h"

#include <sstream>

geng::sdl::SDLRendering::SDLRendering(const char* pWindowTitle, int windowX, int windowY)
	:BaseGameComponent("SDLRendering"),
	m_windowTitle(pWindowTitle),
	m_windowX(windowX),
	m_windowY(windowY)
{ }

bool geng::sdl::SDLRendering::SDLRendering::Initialize(const std::shared_ptr<IGame>& pGame)
{

	// Create SDL resources
	m_pWindow = sdl::CreateSDLObj<SDL_Window>(m_windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_windowX, m_windowY, SDL_WINDOW_SHOWN);

	if (!m_pWindow)
	{
		std::stringstream ssm;
		ssm << "SDLRendering: could not create window; SDL error " << SDL_GetError();
		std::string sErr = ssm.str();
		pGame->LogError(sErr.c_str());
		return false;
	}

	m_pRenderer = sdl::CreateSDLObj<SDL_Renderer>(m_pWindow.get(), -1, SDL_RENDERER_ACCELERATED);

	if (!m_pRenderer)
	{
		std::stringstream ssm;
		ssm << "SDLRendering: could not create renderer; SDL error " << SDL_GetError();
		std::string sErr = ssm.str();
		pGame->LogError(sErr.c_str());
		return false;
	}

	return true;
}