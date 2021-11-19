#include "ColumnsSDLRenderer.h"

#include "SDLHelpers.h"

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

	return true;
}

void geng::columns::ColumnsSDLRenderer::RenderSquareAt(int x, int y,
	geng::sdl::RGBA color)
{
	int renderShadow = 4;   // 3 pixels

	// First draw the shadow, then draw the boss, then draw the main square
	sdl::RGBA colorShadow(color.red - color.red / 20,
		color.blue - color.blue / 20,
		color.green - color.green / 20,
		SDL_ALPHA_OPAQUE);

	SDL_Rect rectShadow{x,y, m_squareSize, m_squareSize};
	sdl::SetDrawColor(m_pRenderer.get(), colorShadow);
	SDL_RenderFillRect(m_pRenderer.get(), &rectShadow);

	sdl::RGBA colorBoss(color.red + color.red / 10,
		color.blue + color.blue / 10,
		color.green + color.green / 10,
		SDL_ALPHA_OPAQUE);
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

	// Draw the board

}

void geng::columns::ColumnsSDLRenderer::Measure()
{
	// Compute the residue
	unsigned int visibleY = m_boardY - m_boardYOffset;
	m_boardBottom = m_windowX % visibleY;

	// Compute square size
	m_squareSize = m_windowY / visibleY;

	// Compute game rectangle dimensions
	unsigned int gameRectW = m_boardX / m_squareSize;
	unsigned int gameRectH = visibleY / m_squareSize;  // approximately window size (to within one square)

	m_boardArea.x = (m_windowX - gameRectW) / 2;
	m_boardArea.y = 0;
	m_boardArea.w = gameRectW;
	m_boardArea.h = gameRectH;

	// One square down and one square behind from the board should be the "predictor"
	m_predictorX = m_boardArea.x - m_squareSize;
	m_predictorY = m_squareSize;


}