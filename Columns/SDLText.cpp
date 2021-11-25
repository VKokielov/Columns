#include "SDLText.h"

bool geng::sdl::Text::SetFont(const std::shared_ptr<TTFResource>& pFont)
{
	m_pFont = pFont;
	m_changeMade = true;
	return SetText(m_text.c_str(), nullptr);
}

void geng::sdl::Text::ResetFont()
{
	m_pFont.reset();
}

bool geng::sdl::Text::SetColor(const RGBA& color)
{
	m_color.r = color.red;
	m_color.g = color.green;
	m_color.b = color.blue;
	m_color.a = color.alpha;
	m_changeMade = true;
	return SetText(m_text.c_str(), nullptr);
}

bool geng::sdl::Text::SetText(const char* pText, SDL_Renderer* pRenderer,
	TextQuality quality)
{
	if (!m_changeMade 
		&& m_text == pText)
	{
		// Nothing to change!
		return true;
	}
	m_changeMade = false;

	// No font
	if (!m_pFont)
	{
		return false;
	}

	// Clear the previous resources
	m_pSurface.reset();
	m_pTexture.reset();

	m_text = pText;

	if (!m_text.empty())
	{	
		if (quality == TextQuality::Rough)
		{
			m_pSurface = CreateSDLObj<SDL_Surface>(TTF_RenderText_Solid,
				m_pFont->GetFont(), m_text.c_str(),
				m_color);
		}
		else if (quality == TextQuality::Nice)
		{
			m_pSurface = CreateSDLObj<SDL_Surface>(TTF_RenderText_Blended,
				m_pFont->GetFont(), m_text.c_str(),
				m_color);
		}
		else
		{
			return false;
		}
											   
		if (!m_pSurface)
		{
			// Reset text to empty
			m_text = "";
			return false;
		}

		m_width = m_pSurface->w;
		m_height = m_pSurface->h;

		if (pRenderer)
		{
			m_pTexture = CreateSDLObj<SDL_Texture>(SDL_CreateTextureFromSurface,
				pRenderer,
				m_pSurface.get());
		}
		else
		{
			m_pTexture.reset();
		}
	}

	return true;
}

bool geng::sdl::Text::RenderTo(SDL_Renderer* pRenderer, int xCoord, int yCenter, 
	                           int maxWidth, int maxHeight,
							   TextAlignment align)
{
	if (m_text.empty() || !m_pSurface)
	{
		return false;
	}

	if (!m_pTexture)
	{
		m_pTexture = CreateSDLObj<SDL_Texture>(SDL_CreateTextureFromSurface,
			pRenderer,
			m_pSurface.get());
		if (!m_pTexture)
		{
			return false;
		}
	}

	// The idea is to render the text such that its center point is at (xCenter,yCenter),
	// and it fits within the bounding rectangle on the destination

	if ((maxHeight != 0 && m_height > maxHeight) 
	|| (maxWidth != 0 && m_width > maxWidth))
	{
		return false;
	}

	// The meaning of the y coordinate is always "centerline of text"
	// The meaning of the x coordinate varies depending on the alignment
	SDL_Rect destRect;
	
	destRect.w = m_width;
	destRect.h = m_height;

	destRect.y = yCenter - m_height / 2;
	if (align == TextAlignment::Left)
	{
		destRect.x = xCoord;
	}
	else if (align == TextAlignment::Center)
	{
		destRect.x = xCoord - m_width / 2;
	}
	else if (align == TextAlignment::Right)
	{
		destRect.x = xCoord - m_width;
	}
	
	return SDL_RenderCopyEx(pRenderer, m_pTexture.get(), nullptr, &destRect, 0.0, nullptr, SDL_FLIP_NONE) == 0;
}