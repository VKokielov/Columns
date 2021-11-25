#pragma once

#include <SDL.h>
#include <string>
#include "BaseGameComponent.h"

namespace geng::sdl
{
	class SDLRendering : public BaseGameComponent
	{
	public:
		SDLRendering(const char* pWindowTitle, 
			int windowX, 
			int windowY);
		bool Initialize(const std::shared_ptr<IGame>& pGame) override;

		const std::shared_ptr<SDL_Window>& GetWindow()
		{
			return m_pWindow;
		}

		const std::shared_ptr<SDL_Renderer>& GetRenderer()
		{
			return m_pRenderer;
		}

		int GetWindowX() const { return m_windowX; }
		int GetWindowY() const { return m_windowY; }
	private:
		std::string m_windowTitle;
		std::shared_ptr<SDL_Window>     m_pWindow;
		std::shared_ptr<SDL_Renderer>   m_pRenderer;
		int m_windowX{};
		int m_windowY{};
	};
}