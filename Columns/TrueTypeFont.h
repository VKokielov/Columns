#pragma once

#include "BaseResource.h"
#include "ResourceLoader.h" // to specialize the template traits
#include "RawMemoryResource.h"
#include "SDLHelpers.h"

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
		TTFResource(TTF_Font* pFont, 
			const std::shared_ptr<RawMemoryResource>& pResource);

		~TTFResource();

		static const char* GetTypeName()
		{
			return "TrueTypeFont";
		}

		TTF_Font* GetFont() { return m_pFont; }

	private:
		TTF_Font* m_pFont;
		// This is needed if we manage the memory ourselves
		std::shared_ptr<RawMemoryResource>   m_pFontRaw;
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

// Define the value of the ResourceTraits for TTFResource (for the resource loader's helper function, see ResourceLoader.h)
namespace geng
{
	template<>
	struct ResourceTraits<sdl::TTFResource>
	{
		using TArgs = sdl::FontArgs;
	};
}