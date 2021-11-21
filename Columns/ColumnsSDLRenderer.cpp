#include "ColumnsSDLRenderer.h"

#include "SDLHelpers.h"
#include "ResourceLoader.h"
#include "RawMemoryResource.h"

#include <sstream>

geng::columns::ColumnsSDLRenderer::ColumnsSDLRenderer(const ColumnsRenderArgs& args)
	:BaseGameComponent("ColumnsSDLRenderer", GameComponentType::IO),
	m_windowX(args.windowX), m_windowY(args.windowY)
{ 
	m_colorMap.emplace(RED, sdl::RGBA(135, 16, 0, SDL_ALPHA_OPAQUE));
	m_colorMap.emplace(GREEN, sdl::RGBA(0,135,47,SDL_ALPHA_OPAQUE));
	m_colorMap.emplace(YELLOW, sdl::RGBA(189, 173, 0, SDL_ALPHA_OPAQUE));
	m_colorMap.emplace(MAGENTA, sdl::RGBA(145, 0, 189, SDL_ALPHA_OPAQUE));
	m_colorMap.emplace(BLUE, sdl::RGBA(0, 44, 189, SDL_ALPHA_OPAQUE));
}

// This is used to avoid RTTI casts
geng::IFrameListener* geng::columns::ColumnsSDLRenderer::GetFrameListener()
{
	return this;
}

std::shared_ptr<geng::sdl::TTFResource> geng::columns::ColumnsSDLRenderer::InitializeFont(geng::IGame* pGame, ResourceLoader* pLoader,
	int pointSize)
{
	std::shared_ptr<sdl::TTFResource> pFont;

	const char* pLoadedFilepath = "c:\\windows\\fonts\\calibri.ttf";
	auto pResourceFile = LoadResource<RawMemoryResource>(pLoader, pLoadedFilepath);

	if (!pResourceFile)
	{
		pGame->LogError("Could not find Calibri font in Windows directory -- trying free variant!");
		pLoadedFilepath = "SourceSansPro-Regular.ttf";
		pResourceFile = LoadResource<RawMemoryResource>(pLoader, pLoadedFilepath);

		if (!pResourceFile)
		{
			pGame->LogError("ColumnsSDLRenderer: Missing Calibri font on Windows and free font file in game directory.");
			return false;
		}
	}

	sdl::FontArgs fontArgs;
	fontArgs.pointSize = pointSize;
	pFont = LoadResource<sdl::TTFResource>(pLoader, pResourceFile, fontArgs);
	if (!pFont)
	{
		std::stringstream ssm;
		ssm << "ColumnsSDLRenderer: could not process TTF file " 
			<< pLoadedFilepath << ", error: " << pLoader->GetResourceLoadError();
		std::string sErr = ssm.str();
		pGame->LogError(sErr.c_str());
	}

	return pFont;
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

	// Create SDL resources
	m_pWindow = sdl::CreateSDLObj<SDL_Window>("Columns", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_windowX, m_windowY, SDL_WINDOW_SHOWN);

	if (!m_pWindow)
	{
		std::stringstream ssm;
		ssm << "ColumnsSDLRenderer: could not create window; SDL error " << SDL_GetError();
		std::string sErr = ssm.str();
		pGame->LogError(sErr.c_str());
		return false;
	}

	m_pRenderer = sdl::CreateSDLObj<SDL_Renderer>(m_pWindow.get(), -1, SDL_RENDERER_ACCELERATED);

	if (!m_pRenderer)
	{
		std::stringstream ssm;
		ssm << "ColumnsSDLRenderer: could not create renderer; SDL error " << SDL_GetError();
		std::string sErr = ssm.str();
		pGame->LogError(sErr.c_str());
		return false;
	}

	// Get board information
	Point boardSize = m_pSim->GetBoardSize();
	m_boardX = boardSize.x;
	m_boardY = boardSize.y;

	// Space for one column from above
	m_boardYOffset = m_pSim->GetColumnSize();

	Measure();

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

	m_scoreLabel.SetFont(pFontLabel);
	m_scoreLabel.SetText("gems", m_pRenderer.get());
	m_levelLabel.SetFont(pFontLabel);
	m_scoreLabel.SetText("level", m_pRenderer.get());

	m_score.SetFont(pFontValue);
	m_level.SetFont(pFontValue);

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

void geng::columns::ColumnsSDLRenderer::RenderContentsAt(int x, int y,
	GridContents gc)
{
	auto itColor = m_colorMap.find(gc);

	// To avoid crashes, default to drawing invalid values as black
	if (itColor != m_colorMap.end())
	{
		RenderSquareAt(x, y, itColor->second);
	}
	else
	{
		static sdl::RGBA black{ 0,0,0,SDL_ALPHA_OPAQUE };
		RenderFullAt(x, y, black);
	}
}

void geng::columns::ColumnsSDLRenderer::OnFrame(IFrameManager* pManager)
{
	SDL_SetRenderDrawColor(m_pRenderer.get(), 17, 23, 64, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(m_pRenderer.get());

	// Draw the board as a black rectangle
	SDL_SetRenderDrawColor(m_pRenderer.get(), 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(m_pRenderer.get(), &m_boardArea);

	// Draw the predicting gems next to the board
	const std::vector<GridContents>& nextGems = m_pSim->GetNextColors();

	int xRect = m_predictorX;
	int yRect = m_predictorY;
	for (GridContents gem : nextGems)
	{
		RenderContentsAt(xRect, yRect, gem);
		yRect += m_squareSize;
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

	constexpr int TEXT_COLUMN_GAP = 20;

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

	m_pSim->IterateGrid(gridRender, m_pSim->PointToIndex(xOrigin));

	SDL_RenderPresent(m_pRenderer.get());
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

	m_boardArea.x = (m_windowX - gameRectW) / 2;
	m_boardArea.y = 0;
	m_boardArea.w = gameRectW;
	m_boardArea.h = gameRectH;

	// One square down and one square behind from the board should be the "predictor"
	m_predictorX = m_boardArea.x - 2 * m_squareSize;
	m_predictorY = m_squareSize;


}