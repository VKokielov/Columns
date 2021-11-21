#include "TrueTypeFont.h"
#include "RawMemoryResource.h"
#include "SDLHelpers.h"

geng::sdl::TTFResource::TTFResource(TTF_Font* pFont)
	:BaseResource(TTFResource::GetTypeName()),
	m_pFont(pFont)
{
}

geng::sdl::TTFFactory::TTFFactory()
	:BaseResourceFactory(TTFResource::GetTypeName())
{

}

bool geng::sdl::TTFFactory::IsMyResource(const ResourceSource& source) const
{
	return (source.GetSourceType() == ResourceSourceType::Filename)
	   || (source.GetSourceType() == ResourceSourceType::ResourceObj
		   && source.GetSource()->GetType() == RawMemoryResource::GetTypeName())
		&& source.GetResourceType() == BaseResourceFactory::GetTypeAsString();
}

std::shared_ptr<geng::IResource> geng::sdl::TTFFactory
::LoadResource(const ResourceSource& source, const ResourceArgs& args,
	std::string& rErr)
{
	TTF_Font* prawFont{ nullptr };

	const FontArgs& typedArgs
		= static_cast<const FontArgs&> (args);

	std::shared_ptr<TTFResource> pFont;

	if (source.GetSourceType() == ResourceSourceType::Filename)
	{
		prawFont = TTF_OpenFont(source.GetPath(), typedArgs.pointSize);
	}
	else if (source.GetSource()->GetType() 
		== RawMemoryResource::GetTypeName())
	{
		auto pMem = std::static_pointer_cast<RawMemoryResource>(source.GetSource());

		std::unique_ptr<SDL_RWops, SDLRWDeleter> pRWOps(SDL_RWFromMem(pMem->GetData(), pMem->GetSize()));

		if (!pRWOps)
		{
			rErr = "Could not construct SDL wrapper around memory";
			return pFont;
		}

		prawFont = TTF_OpenFontRW(pRWOps.get(), 0, typedArgs.pointSize);
	}

	if (!prawFont)
	{
		rErr = "TTF engine failed to create font, error: ";
		rErr += TTF_GetError();
		return pFont;
	}
	
	pFont.reset(new TTFResource(prawFont));
	return pFont;
}