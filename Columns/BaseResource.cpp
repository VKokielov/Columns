#include "BaseResource.h"

const char* geng::BaseResource::GetType() const
{
	return m_resType.c_str();
}

const char* geng::BaseResourceFactory::GetType() const
{
	return m_resType.c_str();
}

bool geng::BaseResourceFactory::IsMyResource(const ResourceSource& source) const
{
	return false;
}
std::shared_ptr<geng::IResource> geng::BaseResourceFactory::LoadResource(const ResourceSource& source,
	const ResourceArgs& args, std::string& rErr)
{
	rErr = "Override LoadResource() function in derived class";

	return std::shared_ptr<geng::IResource>();
}