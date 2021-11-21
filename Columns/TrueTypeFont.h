#pragma once

#include "BaseResource.h"

#include <SDL.h>
#include <SDL_ttf.h>

namespace geng::sdl
{

	struct FontArgs : public ResourceArgs
	{
		int pointSize;
	};

	class TTFResource : public BaseResource
	{
	public:
		TTFResource(TTF_Font* pFont);

		static const char* GetTypeName();

		TTF_Font* GetFont() { return m_pFont; }

	private:
		TTF_Font* m_pFont;
	};

	class TTFFactory : public BaseResourceFactory
	{
	public:
		TTFFactory();

		bool IsMyResource(const ResourceSource& source) const override;
		std::shared_ptr<IResource> LoadResource(const ResourceSource& source,
			const ResourceArgs& args, std::string& rErr) override;

	private:
		size_t m_bufferSize;
	};



}