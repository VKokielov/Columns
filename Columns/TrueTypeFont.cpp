#include "TrueTypeFont.h"
#include "RawMemoryResource.h"
#include "SDLHelpers.h"

geng::sdl::TTFResource::TTFResource(TTF_Font* pFont, 
	const std::shared_ptr<geng::RawMemoryResource>& pFontRaw)
	:BaseResource(TTFResource::GetTypeName()),
	m_pFont(pFont),
	m_pFontRaw(pFontRaw)
{
}

geng::sdl::TTFResource::TTFResource(TTF_Font* pFont)
	:BaseResource(TTFResource::GetTypeName()),
	m_pFont(pFont)
{ }

geng::sdl::TTFResource::~TTFResource()
{
	TTF_CloseFont(m_pFont);
}

geng::sdl::TTFFactory::TTFFactory()
	:BaseResourceFactory(TTFResource::GetTypeName())
{

}

bool geng::sdl::TTFFactory::IsMyResource(const ResourceSource& source) const
{
	bool isValidSourceObj = (source.GetSourceType() == ResourceSourceType::Filename)
		|| (source.GetSourceType() == ResourceSourceType::ResourceObj
			&& 
			!strcmp(source.GetSource()->GetType(), RawMemoryResource::GetTypeName()));

	bool isValidType = source.GetResourceType() == BaseResourceFactory::GetTypeAsString();

	return isValidSourceObj && isValidType;
}

std::shared_ptr<geng::IResource> geng::sdl::TTFFactory
::LoadResource(const ResourceSource& source, const ResourceArgs& args,
	std::string& rErr)
{
	TTF_Font* pfontHeader{ nullptr };

	const FontArgs& typedArgs
		= static_cast<const FontArgs&> (args);

	std::shared_ptr<TTFResource> pFont;
	std::shared_ptr<RawMemoryResource> pFontRaw;

	if (source.GetSourceType() == ResourceSourceType::Filename)
	{
		pfontHeader = TTF_OpenFont(source.GetPath(), typedArgs.pointSize);

		if (pfontHeader)
		{
			pFont.reset(new TTFResource(pfontHeader));
		}
	}
	else if (!strcmp(source.GetSource()->GetType(), RawMemoryResource::GetTypeName()))
	{
		pFontRaw = std::static_pointer_cast<RawMemoryResource>(source.GetSource());

		std::unique_ptr<SDL_RWops, SDLRWDeleter> pRWOps(SDL_RWFromConstMem(pFontRaw->GetData(), (int)pFontRaw->GetSize()));

		if (!pRWOps)
		{
			rErr = "Could not construct SDL wrapper around memory";
			return pFont;
		}

		pfontHeader = TTF_OpenFontRW(pRWOps.get(), 1, typedArgs.pointSize);
		//TTF_CloseFont(pfontHeader);

		if (pfontHeader)
		{
			pRWOps.release();
			pFont.reset(new TTFResource(pfontHeader, pFontRaw));
		}
	}

	if (!pFont)
	{
		rErr = "TTF engine failed to create font, error: ";
		rErr += TTF_GetError();
		return pFont;
	}
	
	return pFont;
}