#include "ColumnsSDLRenderer.h"

#include "SDLHelpers.h"
#include "ResourceLoader.h"
#include "RawMemoryResource.h"
#include "SDLRendering.h"
#include "DTSimple.h"
#include "ResDescriptor.h"

#include <sstream>

geng::columns::ColumnsSDLRenderer::ColumnsSDLRenderer(const ColumnsRenderArgs& args)
	:BaseGameComponent("ColumnsSDLRenderer")
{ 
	m_colorMap.emplace(RED, sdl::RGBA(135, 16, 0, SDL_ALPHA_OPAQUE));
	m_colorMap.emplace(GREEN, sdl::RGBA(0,135,47,SDL_ALPHA_OPAQUE));
	m_colorMap.emplace(YELLOW, sdl::RGBA(189, 173, 0, SDL_ALPHA_OPAQUE));
	m_colorMap.emplace(MAGENTA, sdl::RGBA(145, 0, 189, SDL_ALPHA_OPAQUE));
	m_colorMap.emplace(BLUE, sdl::RGBA(0, 44, 189, SDL_ALPHA_OPAQUE));
}

std::shared_ptr<geng::sdl::TTFResource> geng::columns::ColumnsSDLRenderer::InitializeFont(geng::IGame* pGame, ResourceLoader* pLoader,
	int pointSize)
{
	using namespace data;

	// Keep a reference to the font typeface name in order to be able to change it if loading
	// fails
	auto pFontName
		= DTElem<simple::Suite>("calibri");

	auto pFontDescriptor =
		DTDict<simple::Suite>
		({
		   {res_desc::RES_TYPE,
				DTElem<simple::Suite>(sdl::TTFResource::GetTypeName())
		   },
		   {"size",
				DTElem<simple::Suite>((int32_t)pointSize)
	       },
		   {"typeface",
				pFontName
		   }
			});

	std::shared_ptr<IResource> pFont =
		pLoader->LoadResource(*pFontDescriptor);

	if (!pFont)
	{
		pGame->LogError("Could not load Calibri font -- trying free variant!");

		std::string errNormalFont = pLoader->GetResourceLoadError();

		// Change the typeface name directly in the descriptor
		pFontName->Set("SourceSansPro-Regular");
		pFont = pLoader->LoadResource(*pFontDescriptor);

		if (!pFont)
		{
			pGame->LogError("ColumnsSDLRenderer: Missing Calibri font and free font file in game directory. Errors:");
			pGame->LogError(errNormalFont.c_str());
			pGame->LogError(pLoader->GetResourceLoadError());
			return std::shared_ptr<geng::sdl::TTFResource>();
		}
	}

	return std::static_pointer_cast<sdl::TTFResource>(pFont);
}

bool geng::columns::ColumnsSDLRenderer::Initialize(const std::shared_ptr<IGame>& pGame)
{
	GetComponentResult getResult;
	m_pSim = GetComponentAs<ColumnsSim>(pGame.get(), "ColumnsSim", getResult);

	if (!m_pSim)
	{
		pGame->LogError("ColumnsSDLRenderer: could not get ColumnsSim");
		return false;
	}

	auto pRendering = GetComponentAs<sdl::SDLRendering>(pGame.get(), "SDLRendering", getResult);

	if (!pRendering)
	{
		pGame->LogError("ColumnsSDLRenderer: could not get SDLRendering component");
		return false;
	}

	m_pWindow = pRendering->GetWindow();
	m_pRenderer = pRendering->GetRenderer();
	m_windowX = pRendering->GetWindowX();
	m_windowY = pRendering->GetWindowY();

	// Initialize the font and texts
	// Get the resource loader
	auto pLoader = GetComponentAs<ResourceLoader>(pGame.get(), "ResourceLoader");

	if (!pLoader)
	{
		pGame->LogError("ColumnsSDLRenderer: could not find ResourceLoader");
		return false;
	}

	constexpr int FONT_SIZE_LABEL = 24;
	constexpr int FONT_SIZE_VALUE = 28;
	constexpr int FONT_SIZE_BANNER = 48;

	std::shared_ptr<sdl::TTFResource> pFontLabel = InitializeFont(pGame.get(), pLoader.get(), 
		FONT_SIZE_LABEL);

	if (!pFontLabel)
	{
		pGame->LogError("Error initializing label font");
		return false;
	}

	std::shared_ptr<sdl::TTFResource> pFontValue = InitializeFont(pGame.get(), pLoader.get(),
		FONT_SIZE_VALUE);
	if (!pFontValue)
	{
		pGame->LogError("Error initializing value font");
		return false;
	}

	std::shared_ptr<sdl::TTFResource> pFontBanner = InitializeFont(pGame.get(), pLoader.get(),
		FONT_SIZE_BANNER);
	if (!pFontValue)
	{
		pGame->LogError("Error initializing banner font");
		return false;
	}

	m_scoreLabel.SetFont(pFontLabel);
	m_scoreLabel.SetText("gems", m_pRenderer.get(), sdl::TextQuality::Nice);
	m_levelLabel.SetFont(pFontLabel);
	m_levelLabel.SetText("level", m_pRenderer.get(), sdl::TextQuality::Nice);
	m_pauseLabel.SetFont(pFontBanner);
	m_pauseLabel.SetText("PAUSED", m_pRenderer.get(), sdl::TextQuality::Nice);
	m_gameOverLabel.SetFont(pFontBanner);
	m_gameOverLabel.SetText("Game over!", m_pRenderer.get(), sdl::TextQuality::Nice);
	m_pressSpaceLabel.SetFont(pFontBanner);
	m_pressSpaceLabel.SetText("Press SPACE to start", m_pRenderer.get(), sdl::TextQuality::Nice);
	m_cheatAcceptedLabel.SetFont(pFontBanner);
	m_cheatAcceptedLabel.SetText("Cheat ACCEPTED", m_pRenderer.get(), sdl::TextQuality::Nice);

	m_score.SetFont(pFontValue);
	m_level.SetFont(pFontValue);

	// Compute the phase length based on the simulation properties
	const GameArgs& gameArgs = pGame->GetGameArgs();
	m_magicAnimation.SetArguments(AnimationArgsForTime(MAGIC_PHASE_COUNT, gameArgs.msTimePerFrame,
		MAGIC_TOTAL_MS));

	m_screenFadeAnimation.SetArguments(AnimationArgsForTime(127, gameArgs.msTimePerFrame,
		FADE_TOTAL_MS));

	return true;
}

void geng::columns::ColumnsSDLRenderer::RenderSquareAt(int x, int y,
	geng::sdl::RGBA color)
{
	int renderShadow = 4;   // 3 pixels

	// First draw the shadow, then draw the boss, then draw the main square

	sdl::RGBA colorShadow = ChangeBrightness<-1,5>(color);

	SDL_Rect rectShadow{x,y, m_squareSize, m_squareSize};
	sdl::SetDrawColor(m_pRenderer.get(), colorShadow);
	SDL_RenderFillRect(m_pRenderer.get(), &rectShadow);

	sdl::RGBA colorBoss = ChangeBrightness<1, 10>(color);

	SDL_Rect rectBoss{ x,y, m_squareSize - renderShadow, m_squareSize - renderShadow };
	sdl::SetDrawColor(m_pRenderer.get(), colorBoss);
	SDL_RenderFillRect(m_pRenderer.get(), &rectBoss);

	SDL_Rect rectBody{ x + renderShadow, y + renderShadow, m_squareSize - 2 * renderShadow, m_squareSize - 2 * renderShadow };
	sdl::SetDrawColor(m_pRenderer.get(), color);
	SDL_RenderFillRect(m_pRenderer.get(), &rectBody);
}

void geng::columns::ColumnsSDLRenderer::RenderFullAt(int x, int y,
	geng::sdl::RGBA color)
{
	// Just draw a solid rectangle, no boss
	// Used for empty spaces

	SDL_Rect rectBody{ x, y, m_squareSize, m_squareSize};
	sdl::SetDrawColor(m_pRenderer.get(), color);
	SDL_RenderFillRect(m_pRenderer.get(), &rectBody);
}

void geng::columns::ColumnsSDLRenderer::RenderClearingAt(int x, int y)
{
	// Create cascading squares
	// TODO:  PRERENDER THIS!!!!!

	static sdl::RGBA black{ 0,0,0, SDL_ALPHA_OPAQUE };
	static sdl::RGBA gold{ 255,163,77, SDL_ALPHA_OPAQUE };

	std::array<sdl::RGBA*, MAGIC_PHASE_COUNT+1> colorArray{ &gold, &black, &black, &black };

	SDL_Rect rectBody{ x, y, m_squareSize, m_squareSize };
	size_t idx = 0;

	int squareDelta = 1;

	if (squareDelta == 0)
	{
		// This is absolutely necessary to prevent an infinite loop
		squareDelta = 1;
	}

	unsigned long arrayPhase = m_magicAnimation.GetCurrentTick();

	while (rectBody.h > 0)
	{
		sdl::RGBA* pColor = colorArray[(arrayPhase + idx) % colorArray.size()];
		sdl::SetDrawColor(m_pRenderer.get(), *pColor);
		SDL_RenderFillRect(m_pRenderer.get(), &rectBody);

		rectBody.h -= std::min(rectBody.h, 2 * squareDelta);
		rectBody.w -= 2 * squareDelta;
		rectBody.x += squareDelta;
		rectBody.y += squareDelta;
		++idx;
	}
}

void geng::columns::ColumnsSDLRenderer::RenderContentsAt(int x, int y,
	GridContents gc)
{
	auto itColor = m_colorMap.find(gc);

	// To avoid crashes, default to drawing invalid values as black
	if (itColor != m_colorMap.end())
	{
		RenderSquareAt(x, y, itColor->second);
	}
	else if (gc == CLEARING)
	{
		RenderClearingAt(x, y);
	}
	else
	{
		static sdl::RGBA black{ 0,0,0,SDL_ALPHA_OPAQUE };
		RenderFullAt(x, y, black);
	}
}

void geng::columns::ColumnsSDLRenderer::OnFrame(const SimState& rSimState,
	const SimContextState* pContextState)
{
	// Phase updates

	// Forward; wrap around
	m_magicAnimation.Step(true, true);

	SDL_SetRenderDrawColor(m_pRenderer.get(), 17, 23, 64, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(m_pRenderer.get());

	// Only draw if initialized
	// TODO:  Make this dependent on the executive instead of the sim?
	if (m_pSim->IsGameInitialized())
	{
		// Draw the board as a black rectangle
		SDL_SetRenderDrawColor(m_pRenderer.get(), 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(m_pRenderer.get(), &m_boardArea);


		// Update the fade to dark animation
		if (m_pausedGame)
		{
			// -> dark
			if (!m_screenFadeAnimation.IsAtEnd())
			{
				//		fprintf(stderr, "\nCurrent ticks %lu\n", m_screenFadeAnimation.GetCurrentTick());
				m_screenFadeAnimation.Step(true, false);
				//		fprintf(stderr, "\nNew ticks %lu\n", m_screenFadeAnimation.GetCurrentTick());
			}
		}
		else
		{
			// -> light
			if (!m_screenFadeAnimation.IsAtStart())
			{
				//		fprintf(stderr, "\nCurrent ticks rev %lu\n", m_screenFadeAnimation.GetCurrentTick());
				m_screenFadeAnimation.Step(false, false);
				//		fprintf(stderr, "\nNew ticks rev %lu\n", m_screenFadeAnimation.GetCurrentTick());
			}
		}

		// Draw the predicting gems next to the board
		const std::vector<GridContents>& nextGems = m_pSim->GetNextColors();

		int xRect = m_predictorX;
		int yRect = m_predictorY;
		if (!m_pausedGame)
		{
			for (GridContents gem : nextGems)
			{
				RenderContentsAt(xRect, yRect, gem);
				yRect += m_squareSize;
			}
		}
		else
		{
			yRect += m_squareSize * (int)nextGems.size();
		}

		// Set the texts
		constexpr size_t RENDERED_NUMBER_LENGTH = 25;

		char txtScore[RENDERED_NUMBER_LENGTH];
		snprintf(txtScore, sizeof(txtScore), "%u", m_pSim->GetGems());
		m_score.SetText(txtScore, m_pRenderer.get());

		char txtLevel[RENDERED_NUMBER_LENGTH];
		snprintf(txtLevel, sizeof(txtLevel), "%u", m_pSim->GetLevel());
		m_level.SetText(txtLevel, m_pRenderer.get());

		// Draw the score label
		int textX = m_boardArea.x - m_squareSize;
		int textY = yRect + m_squareSize;

		constexpr int TEXT_COLUMN_GAP = 10;

		m_scoreLabel.RenderTo(m_pRenderer.get(), textX, textY, 0, 0, sdl::TextAlignment::Right);
		textY += m_scoreLabel.GetHeight() + TEXT_COLUMN_GAP;
		m_score.RenderTo(m_pRenderer.get(), textX, textY, 0, 0, sdl::TextAlignment::Right);
		textY += m_score.GetHeight() + TEXT_COLUMN_GAP;
		m_levelLabel.RenderTo(m_pRenderer.get(), textX, textY, 0, 0, sdl::TextAlignment::Right);
		textY += m_levelLabel.GetHeight() + TEXT_COLUMN_GAP;
		m_level.RenderTo(m_pRenderer.get(), textX, textY, 0, 0, sdl::TextAlignment::Right);

		// Draw the board
		Point xOrigin{ 0, m_boardYOffset };

		auto gridRender = [this](const Point& pt, const GridSquare& gsquare)
		{
			// Get the coordinates
			int xSquare = m_boardArea.x + m_squareSize * pt.x;
			int ySquare = m_boardArea.y + m_squareSize * (pt.y - m_boardYOffset);

			GridContents toDraw = gsquare.isVisible ? gsquare.contents : EMPTY;
			RenderContentsAt(xSquare, ySquare, toDraw);
		};

		if (!m_pausedGame && m_inGame)
		{
			m_pSim->IterateGrid(gridRender, m_pSim->PointToIndex(xOrigin));
		}
	}

	// Banner
	if (m_pSim->CheatHappened())
	{
		m_timeHideCheatLabel = rSimState.execSimulatedTime
			+ CHEAT_BANNER_MS;
	}

	unsigned int bannerX = m_windowX / 2;
	unsigned int bannerY = 30;

	if (!m_pausedGame && !m_inGame)
	{
		if (m_pSim->IsGameOver())
		{
			m_gameOverLabel.RenderTo(m_pRenderer.get(), bannerX, bannerY, 0, 0, sdl::TextAlignment::Center);
		}
		else
		{
			m_pressSpaceLabel.RenderTo(m_pRenderer.get(), bannerX, bannerY, 0, 0, sdl::TextAlignment::Center);
		}
	}
	else if (m_timeHideCheatLabel != 0 &&
		m_timeHideCheatLabel >= rSimState.execSimulatedTime)
	{
		m_cheatAcceptedLabel.RenderTo(m_pRenderer.get(), bannerX, bannerY, 0, 0, sdl::TextAlignment::Center);
	}

	// Draw the "curtain"
	if (!m_screenFadeAnimation.IsAtStart())
	{
		Uint8 uiAlpha = (Uint8)m_screenFadeAnimation.GetCurrentTick();
		//fprintf(stderr, "Filling rect %hhu...\n", uiAlpha);

		SDL_SetRenderDrawBlendMode(m_pRenderer.get(), SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(m_pRenderer.get(), 0, 0, 0, uiAlpha);
		SDL_RenderFillRect(m_pRenderer.get(), &m_curtainArea);
		SDL_SetRenderDrawBlendMode(m_pRenderer.get(), SDL_BLENDMODE_NONE);
	}

	if (m_pausedGame)
	{
		m_pauseLabel.RenderTo(m_pRenderer.get(), bannerX, bannerY, 0, 0, sdl::TextAlignment::Center);
	}

	SDL_RenderPresent(m_pRenderer.get());
}

void geng::columns::ColumnsSDLRenderer::OnStartGame()
{
	// Get board information
	Point boardSize = m_pSim->GetBoardSize();
	m_boardX = boardSize.x;
	m_boardY = boardSize.y;

	// Space for one column from above
	m_boardYOffset = m_pSim->GetColumnSize();

	Measure();
	
	m_inGame = true;
}

void geng::columns::ColumnsSDLRenderer::OnEndGame() 
{ 
	m_inGame = false;
}

void geng::columns::ColumnsSDLRenderer::OnPauseGame(bool pauseState)
{
	m_pausedGame = pauseState;
}

void geng::columns::ColumnsSDLRenderer::Measure()
{
	// Compute the residue
	unsigned int visibleY = m_boardY - m_boardYOffset;
	m_boardBottom = m_windowY - (m_windowY % visibleY);

	// Compute square size
	m_squareSize = m_boardBottom / visibleY;

	// Compute game rectangle dimensions
	unsigned int gameRectW = m_squareSize * m_boardX;
	unsigned int gameRectH = m_squareSize * visibleY;  // approximately window size (to within one square)

	m_curtainArea.x = 0;
	m_curtainArea.y = 0;
	m_curtainArea.w = m_windowX;
	m_curtainArea.h = m_windowY;

	m_boardArea.x = (m_windowX - gameRectW) / 2;
	m_boardArea.y = 0;
	m_boardArea.w = gameRectW;
	m_boardArea.h = gameRectH;

	// One square down and one square behind from the board should be the "predictor"
	m_predictorX = m_boardArea.x - 2 * m_squareSize;
	m_predictorY = m_squareSize;


}