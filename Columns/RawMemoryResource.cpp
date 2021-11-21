#include "RawMemoryResource.h"

#include <cstdio>
#include <filesystem>
#include <sstream>

const char* geng::RawMemoryResource::GetTypeName()
{
	return "RawMemResource";
}

geng::RawMemoryResource::RawMemoryResource(std::unique_ptr<uint8_t[]>&& pMem, size_t size)
	:BaseResource(RawMemoryResource::GetTypeName()),
	m_pMem(std::move(pMem)),
	m_size(size)
{ 
}

geng::RawMemoryFactory::RawMemoryFactory(size_t bufferSize)
	:BaseResourceFactory(RawMemoryResource::GetTypeName()),
	m_bufferSize(bufferSize)
{

}

bool geng::RawMemoryFactory::IsMyResource(const ResourceSource& source) const
{
	return source.GetSourceType() == ResourceSourceType::Filename
		&& source.GetResourceType() == BaseResourceFactory::GetTypeAsString();
}
std::shared_ptr<geng::IResource> geng::RawMemoryFactory
	::LoadResource(const ResourceSource& source, const ResourceArgs& args,
		std::string& rErr)
{
	std::filesystem::path fPath(source.GetPath());

	std::shared_ptr<RawMemoryResource> pRet;
	
	if (!std::filesystem::exists(fPath))
	{
		std::stringstream ssmError;
		ssmError << "File " << fPath.c_str() << " doesn't exist in the filesystem.";
		rErr = ssmError.str();
		return pRet;  // implicit upcast
	}

	auto fileSize = std::filesystem::file_size(fPath);
	// TODO:  File size hard limits??

	std::unique_ptr<uint8_t[]>  pFileData{ new uint8_t[fileSize] };
	size_t fptr{ 0 };

	FILE* pFile = fopen(source.GetPath(), "rb");
	if (!pFile)
	{
		std::stringstream ssmError;
		ssmError << "File " << fPath.c_str() << " could not be opened by C stdio";
		rErr = ssmError.str();
		return pRet;  // implicit upcast
	}

	bool goRead{ true };
	while (goRead)
	{
		size_t nRead = fread(pFileData.get() + fptr, 1, m_bufferSize, pFile);
		if (nRead > 0)
		{
			fptr += nRead;
		}
		else
		{
			goRead = false;
		}
	}

	pRet.reset(new RawMemoryResource(std::move(pFileData), fileSize));
	return pRet;
}