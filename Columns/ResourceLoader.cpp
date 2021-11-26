#include "ResourceLoader.h"

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
std::shared_ptr<geng::IResource> geng::ResourceType::LoadResource(const ResourceSource& rloc,
	const ResourceArgs& args,
	std::string& rErr)
{
	// Go through all available factories until one appears that can load this resource type
	std::shared_ptr<geng::IResource> pRet{};
	unsigned int timesTried{ 0 };

	std::string errors = "Errors reported:\n";

	std::string facError;
	for (auto& rFactory : m_factories)
	{
		if (rFactory->IsMyResource(rloc))
		{
			facError = "";
			pRet = rFactory->LoadResource(rloc, args, facError);
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

bool geng::ResourceLoader::CreateType(const char* typeName)
{
	std::string sName{ typeName };
	if (m_resourceTypeMap.count(sName) > 0)
	{
		// Already exists
		return true;
	}

	m_resourceTypeMap.emplace(sName, m_resTypes.size());
	m_resTypes.emplace_back();
	return true;
}

bool geng::ResourceLoader::AddFactory(const std::shared_ptr<IResourceFactory>& pFactory)
{
	const char* pType = pFactory->GetType();

	auto itTypeObj = m_resourceTypeMap.find(pType);
	if (itTypeObj == m_resourceTypeMap.end())
	{
		return false;
	}

	m_resTypes[itTypeObj->second].AddFactory(pFactory);
	return true;
}

std::shared_ptr<geng::IResource> geng::ResourceLoader::LoadResource(const geng::ResourceSource& rLoc,
	const geng::ResourceArgs& args)
{
	// Dispatch
	std::shared_ptr<IResource> pRet;

	auto itTypeObj = m_resourceTypeMap.find(rLoc.GetResourceType());
	if (itTypeObj == m_resourceTypeMap.end())
	{
		m_error = "Can't load resource -- type is unknown";
		return false;
	}

	return m_resTypes[itTypeObj->second].LoadResource(rLoc, args, m_error);
}