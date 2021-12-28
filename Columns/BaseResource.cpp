#include "BaseResource.h"

const char* geng::BaseResourceFactory::GetType() const
{
	return m_resType.c_str();
}

bool geng::BaseResourceFactory::IsMyResource(const data::IDatum& resourceDesc) const
{
	return false;
}
std::shared_ptr<geng::IResource> geng::BaseResourceFactory::LoadResource(const data::IDatum& resourceDesc,
	IResourceLoader& loader,
	std::string& rErr)
{
	rErr = "Override LoadResource() function in derived class";

	return std::shared_ptr<geng::IResource>();
}