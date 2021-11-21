#pragma once

#include <SDL.h>
#include "BaseGameComponent.h"
#include "SDLHelpers.h"
#include "ColumnsSim.h"
#include "TrueTypeFont.h"
#include <utility>

namespace geng::columns
{
	struct ColumnsRenderArgs
	{
		unsigned int windowX;
		unsigned int windowY;

		unsigned int renderShadow;
	};

	class ColumnsSDLRenderer : public BaseGameComponent,
							   public IFrameListener
	{
	public:
		ColumnsSDLRenderer(const ColumnsRenderArgs& args);
		bool Initialize(const std::shared_ptr<IGame>& pGame) override;
		void OnFrame(IFrameManager* pManager) override;

		IFrameListener* GetFrameListener() override;
	private:
		template<typename T, T lower, T upper, typename U>
		static void ClampTo(T& value, U delta)
		{
			if (delta < 0)
			{
				if (value < lower-delta)
				{
					value = lower;
				}
				else
				{
					value += delta;
				}
			}
			else
			{
				if (value > upper - delta)
				{
					value = upper;
				}
				else
				{
					value += delta;
				}
			}
		}

		template<int num, int denom>
		static sdl::RGBA ChangeBrightness(const sdl::RGBA& source)
		{
			constexpr int dslice = (num * 255) / denom;

			sdl::RGBA result{ source };
			ClampTo<Uint8, 0,255>(result.red, dslice);
			ClampTo<Uint8, 0,255>(result.green, dslice);
			ClampTo<Uint8, 0,255>(result.blue, dslice);
			// Alpha is not touched

			return result;
		}

		void RenderSquareAt(int x, int y,
			geng::sdl::RGBA color);

		void RenderFullAt(int x, int y,
			geng::sdl::RGBA color);

		void RenderContentsAt(int x, int y,
			GridContents gc);

		void Measure();

		// Color map
		std::unordered_map<GridContents, sdl::RGBA>
			m_colorMap;

		int m_windowX{ 0 }, m_windowY{ 0 };
		unsigned int m_boardX{ 0 }, m_boardY{ 0 };
		unsigned int m_boardYOffset{ 0 };  // The "invisible" part of the board
		int m_predictorX{ 0 }, m_predictorY{ 0 };

		int m_boardLeft{ 0 };
		int m_boardBottom{ 0 };
		int m_squareSize{ 0 };

		SDL_Rect m_boardArea;

		// SDL
		std::shared_ptr<SDL_Window>     m_pWindow;
		std::shared_ptr<SDL_Renderer>   m_pRenderer;

		// Sim
		std::shared_ptr<ColumnsSim>  m_pSim;

		// Font - calibri or failing that, some free variant 
		std::shared_ptr<sdl::TTFResource>  m_pTextFont;

		// Prerendered 
	};


}