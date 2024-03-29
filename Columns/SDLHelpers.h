#pragma once

#include <SDL.h>
#include <memory>
#include <functional>

namespace geng::sdl
{
	template<typename Obj>
	struct SDLObjectTraits;

	template<typename Obj, typename ... Args>
	std::shared_ptr<Obj> CreateSDLObj(Args&&...args)
	{
		using OTraits = SDLObjectTraits<Obj>;
		using TCreator = typename OTraits::TCreator;
		using TDeleter = typename OTraits::TDeleter;

		Obj* pObj = TCreator::Create(std::forward<Args>(args)...);
		if (!pObj)
		{
			return std::shared_ptr<Obj>();
		}

		return std::shared_ptr<Obj>(pObj, TDeleter());
	}


	struct SDLWindowCreator
	{
	public:
		template<typename ... Args>
		static SDL_Window* Create(Args&& ... args)
		{
			return SDL_CreateWindow(std::forward<Args>(args)...);
		}
	};

	struct SDLWindowDeleter
	{
	public:
		void operator()(SDL_Window* pWindow)
		{
			SDL_DestroyWindow(pWindow);
		}
	};

	template<>
	struct SDLObjectTraits<SDL_Window>
	{
		using TCreator = SDLWindowCreator;
		using TDeleter = SDLWindowDeleter;
	};

	struct SDLRendererCreator
	{
	public:
		template<typename ... Args>
		static SDL_Renderer* Create(Args&& ... args)
		{
			return SDL_CreateRenderer(std::forward<Args>(args)...);
		}
	};

	struct SDLRendererDeleter
	{
	public:
		void operator()(SDL_Renderer* pRenderer)
		{
			SDL_DestroyRenderer(pRenderer);
		}
	};

	template<>
	struct SDLObjectTraits<SDL_Renderer>
	{
		using TCreator = SDLRendererCreator;
		using TDeleter = SDLRendererDeleter;
	};

	struct SDLTextureCreator
	{
	public:
		template<typename Func, typename ... Args>
		static SDL_Texture* Create(Func&& func, Args&& ... args)
		{
			return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
		}
	};

	struct SDLTextureDeleter
	{
	public:
		void operator()(SDL_Texture* pTexture)
		{
			SDL_DestroyTexture(pTexture);
		}
	};

	template<>
	struct SDLObjectTraits<SDL_Texture>
	{
		using TCreator = SDLTextureCreator;
		using TDeleter = SDLTextureDeleter;
	};

	struct SDLSurfaceCreator
	{
	public:
		template<typename Func, typename ... Args>
		static SDL_Surface* Create(Func&& func, Args&& ... args)
		{
			return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
		}
	};

	struct SDLSurfaceDeleter
	{
	public:
		void operator()(SDL_Surface* pSurface)
		{
			SDL_FreeSurface(pSurface);
		}
	};

	template<>
	struct SDLObjectTraits<SDL_Surface>
	{
		using TCreator = SDLSurfaceCreator;
		using TDeleter = SDLSurfaceDeleter;
	};

	struct SDLRWDeleter
	{
	public:
		void operator()(SDL_RWops* pops)
		{
			SDL_RWclose(pops);
		}
	};

	struct RGBA
	{
		Uint8  red;
		Uint8  green;
		Uint8  blue;
		Uint8  alpha;

		RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
			:red(r),
			green(g),
			blue(b),
			alpha(a)
		{ }
	};


	inline int SetDrawColor(SDL_Renderer* pRenderer, const RGBA& rgba)
	{
		return SDL_SetRenderDrawColor(pRenderer, rgba.red, rgba.green, rgba.blue, rgba.alpha);
	}
}