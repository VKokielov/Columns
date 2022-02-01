#include "TrueTypeFont.h"
#include "RawMemoryResource.h"
#include "SDLHelpers.h"
#include "DTUtils.h"
#include "DTConstruct.h"
#include "ResDescriptor.h"
#include "DTSimple.h"
#include "PathUtils.h"

#include <sstream>

geng::sdl::TTFResource::TTFResource(const char* pDesc,
	TTF_Font* pFont, 
	const std::shared_ptr<geng::RawMemoryResource>& pFontRaw)
	:BaseResource(TTFResource::GetTypeName(), pDesc ? pDesc : ""),
	m_pFont(pFont),
	m_pFontRaw(pFontRaw)
{
}

geng::sdl::TTFResource::TTFResource(const char* pDesc, TTF_Font* pFont)
	:BaseResource(TTFResource::GetTypeName(), pDesc ? pDesc : ""),
	m_pFont(pFont)
{ }

geng::sdl::TTFResource::~TTFResource()
{
	TTF_CloseFont(m_pFont);
}

geng::sdl::TTFFactory::TTFFactory(const std::vector<std::string>& searchPaths)
	:BaseResourceFactory(TTFResource::GetTypeName()),
	m_searchPaths(searchPaths)
{

}

bool geng::sdl::TTFFactory::IsMyResource(const data::IDatum& resDescriptor) const
{
	std::string resType;
	if (data::GetDictValue(resDescriptor, res_desc::RES_TYPE, resType)
		!= data::AccessResult::OK)
	{
		return false;
	}

	return resType == BaseResourceFactory::GetTypeAsString();
}

std::shared_ptr<geng::IResource> geng::sdl::TTFFactory
::LoadResource(const data::IDatum& resourceDesc,
	IResourceLoader& loader,
	std::string& rErr)
{
	using namespace data;
	std::shared_ptr<TTFResource> pFont;

	// get the font size and the source
	int16_t fontSize{};
	if (GetDictValue(resourceDesc, "size", fontSize) != AccessResult::OK)
	{
		rErr = "descriptor must have a \"size\" field";
		return pFont;
	}
	
	std::shared_ptr<data::IDatum> pTypeface;
	if (GetDictChild(resourceDesc, "typeface", pTypeface) != AccessResult::OK)
	{
		rErr = "descriptor must have a \"typeface\" field";
		return pFont;
	}

	// Get the font as a raw resource
	std::shared_ptr<RawMemoryResource> pFontRaw;
	auto typefaceType = pTypeface->GetDatumType();

	std::string fontDesc;

	if (typefaceType == BaseDatumType::Element)
	{
		// Should be a string
		std::string fontTypeface;
		if (GetValue(*static_cast<IElementDatum*>(pTypeface.get()), 
			fontTypeface) != data::AccessResult::OK)
		{
			rErr = "typeface -- must be a string (or a raw memory resource)";
			return pFont;
		}

		// Try without and with a .ttf extension
		std::string ttfTypeface{ fontTypeface };
		ttfTypeface += ".ttf";

		std::vector<std::string> tfFiles{ fontTypeface, ttfTypeface };

		std::filesystem::path fontPath
			= filepath::FindInSearchPath(m_searchPaths, tfFiles);

		if (fontPath.empty())
		{
			rErr = "could not find typeface in search path";
			return pFont;
		}

		// Load the font as a raw file
		auto pFontDescriptor = 
			DTDict<simple::Suite>
			({ 
			   {res_desc::RES_TYPE,
					DTElem<simple::Suite>(RawMemoryResource::GetTypeName())
			   },
			   {res_desc::SOURCE,
					DTElem<simple::Suite>(fontPath.string().c_str())
			   }
			});

		std::string fontFileLoadError;
		
		auto pLoadedFont = loader.LoadResource(*pFontDescriptor);

		if (!pLoadedFont)
		{
			rErr = "could not load font resource file ";
			rErr += fontPath.string();
			rErr += "; reason: ";
			rErr += fontFileLoadError;
			return pFont;
		}

		{
			std::stringstream ssmDesc;
			ssmDesc << fontTypeface << "|size=" << fontSize << "|path=" << fontPath.string();
			fontDesc = ssmDesc.str();
		}

		pFontRaw = std::static_pointer_cast<RawMemoryResource>(pLoadedFont);
	}
	else if (typefaceType == BaseDatumType::Object)
	{
		std::string objType{static_cast<IObjectDatum*>(pTypeface.get())->GetObjectType() };

		if (objType != RawMemoryResource::GetObjectTypeName())
		{
			rErr = "typeface -- if an object, then must be a raw memory resource";
			return pFont;
		}

		pFontRaw = std::static_pointer_cast<RawMemoryResource>(pTypeface);

		{
			std::stringstream ssmDesc;
			ssmDesc << "<memory-font>|size=" << fontSize;
			fontDesc = ssmDesc.str();
		}
	}

	TTF_Font* pfontHeader{ nullptr };
	std::unique_ptr<SDL_RWops, SDLRWDeleter> pRWOps(SDL_RWFromConstMem(pFontRaw->GetData(), (int)pFontRaw->GetSize()));

	if (!pRWOps)
	{
		rErr = "(font loader) could not construct SDL wrapper around memory";
		return pFont;
	}

	pfontHeader = TTF_OpenFontRW(pRWOps.get(), 1, fontSize);

	if (pfontHeader)
	{
		pRWOps.release();
		pFont.reset(new TTFResource(fontDesc.c_str(), pfontHeader, pFontRaw));
	}
	
	if (!pFont)
	{
		rErr = "TTF engine failed to create font, error: ";
		rErr += TTF_GetError();
		return pFont;
	}
	
	return pFont;
}