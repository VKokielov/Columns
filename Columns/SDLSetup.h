#pragma once

#include <SDL.h>
#include "BaseGameComponent.h"

namespace geng::sdl
{

	struct SDLWindowDeleter
	{
	public:
		void operator()(SDL_Window* pWindow)
		{
			SDL_DestroyWindow(pWindow);
		}
	};


	class SetUp : public BaseGameComponent
	{
	public:
		SetUp(unsigned int xdim, unsigned int ydim)
			:BaseGameComponent("SDLSetUp", GameComponentType::SetUp)
			,m_xDim(xdim), m_yDim(ydim)
		{ }



	private:
		unsigned int m_xDim;
		unsigned int m_yDim;

		std::shared_ptr<SDL_Window>   m_pMainWindow;
	};


}