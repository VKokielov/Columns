#pragma once

#include "TrueTypeFont.h"
#include "SDLHelpers.h"

#include <SDL.h>
#include <string>

namespace geng::sdl
{

	// An encapsulation for text, including the SDL texture and surface associated with it
	// The font needs to be preloaded.

	enum class TextAlignment
	{
		Left,
		Right,
		Center
	};

	enum class TextQuality
	{
		Rough,
		Nice
	};

	class Text
	{
	public:		
		void ResetFont();
		bool SetFont(const std::shared_ptr<TTFResource>& pFont);
		bool SetColor(const RGBA& color);
		bool SetText(const char* pText, SDL_Renderer* pRenderer, 
			TextQuality quality = TextQuality::Rough);
		bool RenderTo(SDL_Renderer* pRenderer, int xCoord, int yCenter, 
					 int maxWidth, int maxHeight, TextAlignment align);

		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }

		const std::shared_ptr<TTFResource>& GetFont() { return m_pFont; }

	private:

		//Args
		std::shared_ptr<TTFResource>     m_pFont;
		SDL_Color m_color{ 255,255,255 };
		std::string m_text{};

		// State
		std::shared_ptr<SDL_Texture>     m_pTexture;
		std::shared_ptr<SDL_Surface>     m_pSurface;
		int m_width{ 0 };
		int m_height{ 0 };
		bool m_changeMade{ false };
	};



}