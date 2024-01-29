#include "RawMemoryResource.h"
#include "DTUtils.h"
#include "ResDescriptor.h"

#include <cstdio>
#include <filesystem>
#include <sstream>

geng::RawMemoryResource::RawMemoryResource(std::unique_ptr<uint8_t[]>&& pMem, 
	size_t size,
	const char* pDesc)
	:BaseResource(RawMemoryResource::GetTypeName(), pDesc ? pDesc : "no-info"),
	m_pMem(std::move(pMem)),
	m_size(size)
{ 
}
geng::RawMemoryResource::RawMemoryResource(const RawMemoryResource& other)
	:BaseResource(RawMemoryResource::GetTypeName(), other.GetRawDescription().c_str()),
	m_size(other.m_size)
{
	m_pMem.reset(new uint8_t[m_size]);
	// Copy the memory
	memcpy(m_pMem.get(), other.m_pMem.get(), m_size);
}

geng::RawMemoryFactory::RawMemoryFactory(size_t bufferSize)
	:BaseResourceFactory(RawMemoryResource::GetTypeName()),
	m_bufferSize(bufferSize)
{

}

bool geng::RawMemoryFactory::IsMyResource(const data::IDatum& resDescriptor) const
{
	std::string resType;
	if (data::GetDictValue(resDescriptor, res_desc::RES_TYPE, resType)
		!= data::AccessResult::OK)
	{
		return false;
	}

	return resType == BaseResourceFactory::GetTypeAsString();
}

std::shared_ptr<geng::IResource> geng::RawMemoryFactory
	::LoadResource(const data::IDatum& resourceDesc,
		IResourceLoader& loader,
		std::string& rErr)
{
	std::shared_ptr<RawMemoryResource> pRet;

	// Get the "source" node
	// It is either an object datum of resource type, or a path specifier
	std::shared_ptr<data::IDatum> pSourceNode;
	if (data::GetDictChild(resourceDesc, res_desc::SOURCE, pSourceNode) != data::AccessResult::OK)
	{
		rErr = "Descriptor missing source";
		return pRet;
	}

	if (pSourceNode->GetDatumType() == data::BaseDatumType::Object)
	{
		std::shared_ptr<data::IObjectDatum> pObjSource 
			{ std::static_pointer_cast<data::IObjectDatum>(pSourceNode) };
		
		std::string objType = pObjSource->GetObjectType();

		if (objType != RawMemoryResource::GetObjectTypeName())
		{
			rErr = "Copy RawMem resource: source is not a resource of the right type";
			return pRet;
		}

		std::shared_ptr<RawMemoryResource> pOther
			{ std::static_pointer_cast<RawMemoryResource>(pObjSource) };

		pRet = std::make_shared<RawMemoryResource>(*pOther);
	}
	else if (pSourceNode->GetDatumType() == data::BaseDatumType::Dictionary)
	{
		std::string filePathProperty;
		if (data::GetDictValue(*pSourceNode, res_desc::FILE_PATH, filePathProperty) != data::AccessResult::OK)
		{
			rErr = "Load RawMem resource: filepath in descriptor missing or not a string";
			return pRet;
		}

		std::filesystem::path fPath(filePathProperty);

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

		FILE* pFile = fopen(filePathProperty.c_str(), "rb");
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

		pRet.reset(new RawMemoryResource(std::move(pFileData), 
			fileSize,
			filePathProperty.c_str()));
	}
	else
	{
		rErr = "Descriptor source format is unrecognized";
	}


	return pRet;
}