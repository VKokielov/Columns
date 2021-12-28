#pragma once

#include "BaseResource.h"
#include "ResourceLoader.h" // to specialize the template traits
#include "RawMemoryResource.h"
#include "SDLHelpers.h"
#include "IFont.h"

#include <SDL.h>
#include <SDL_ttf.h>

namespace geng::sdl
{

	struct FontArgs : public ResourceArgs
	{
		int pointSize;
	};

	class TTFResource : public BaseResource<TTFResource, render::IFont>
	{
	public:
		// There is no reason to copy fonts
		TTFResource(const TTFResource&) = delete;

		TTFResource(const char* pDesc, 
			TTF_Font* pFont);
		TTFResource(const char* pDesc,
			TTF_Font* pFont, 
			const std::shared_ptr<RawMemoryResource>& pResource);

		~TTFResource();

		// Note: this function is STATIC; it is used as a constant
		static const char* GetTypeName()
		{
			return "TrueTypeFont";
		}
		static const char* GetObjectTypeName()
		{
			return "Resource_TrueTypeFont";
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
		TTFFactory(const std::vector<std::string>& searchPaths);

		bool IsMyResource(const data::IDatum& resourceDesc) const override;
		std::shared_ptr<IResource> LoadResource(const data::IDatum& resourceDesc,
			IResourceLoader& loader,
			std::string& rErr) override;

	private:
		std::vector<std::string> m_searchPaths;
		size_t m_bufferSize;
	};
}