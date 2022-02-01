#include "ResourceLoader.h"
#include "DTUtils.h"
#include "ResDescriptor.h"

// ResourceType
void geng::ResourceType::AddFactory(const std::shared_ptr<IResourceFactory>& pFactory)
{
	if (std::find(m_factories.begin(), m_factories.end(), pFactory) != m_factories.end())
	{
		return;
	}

	m_factories.emplace_back(pFactory);
}

// Tries all available loaders until one succeeds
std::shared_ptr<geng::IResource> geng::ResourceType::LoadResource(const data::IDatum& resDescriptor,
	std::string& rErr)
{
	// Go through all available factories until one appears that can load this resource type
	std::shared_ptr<geng::IResource> pRet{};
	unsigned int timesTried{ 0 };

	std::string errors = "Errors reported:\n";

	std::string facError;
	for (auto& rFactory : m_factories)
	{
		if (rFactory->IsMyResource(resDescriptor))
		{
			facError = "";
			pRet = rFactory->LoadResource(resDescriptor, m_owner, facError);
			if (pRet)
			{
				return pRet;
			}

			errors += '\t';
			errors += facError;
			errors += '\n';
		}
		++timesTried;
	}

	rErr = errors;
	if (timesTried == 0)
	{
		rErr = "No resource factories available for this type";
	}
	else
	{
		rErr = "No resource factory for this type could load the resource";
	}

	return pRet;
}

// ResourceLoader
geng::ResourceLoader::ResourceLoader()
	:BaseGameComponent("ResourceLoader")
{ }

bool geng::ResourceLoader::AddFactory(const std::shared_ptr<IResourceFactory>& pFactory)
{
	std::string resType{ pFactory->GetType() };

	auto itTypeObj = m_resourceTypeMap.find(resType);
	if (itTypeObj == m_resourceTypeMap.end())
	{
		itTypeObj = m_resourceTypeMap.emplace(resType, m_resTypes.size()).first;
		m_resTypes.emplace_back(*this);
	}

	m_resTypes[itTypeObj->second].AddFactory(pFactory);
	return true;
}

const char* geng::ResourceLoader::GetResourceLoadError() const
{ 
	return m_error.c_str(); 
}

std::shared_ptr<geng::IResource> geng::ResourceLoader::LoadResource(const geng::data::IDatum& resDescriptor)
{
	// Dispatch
	std::shared_ptr<IResource> pRet;

	// Get resource type
	std::string resType;
	if (data::GetDictValue(resDescriptor, res_desc::RES_TYPE, resType) != data::AccessResult::OK)
	{
		m_error = "Could not get resource type from descriptor";
		return pRet;
	}


	auto itTypeObj = m_resourceTypeMap.find(resType);
	if (itTypeObj == m_resourceTypeMap.end())
	{
		m_error = "Can't load resource -- no factory for this type: ";
		m_error += resType;
		return false;
	}

	return m_resTypes[itTypeObj->second].LoadResource(resDescriptor, m_error);
}