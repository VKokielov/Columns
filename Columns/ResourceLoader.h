#pragma once

#include "BaseGameComponent.h"
#include "Resource.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <cinttypes>
#include <string>

namespace geng
{


	class ResourceType
	{
	public:
		void AddFactory(const std::shared_ptr<IResourceFactory>& pFactory);

		// Tries all available loaders until one succeeds
		std::shared_ptr<IResource> LoadResource(const ResourceSource& rloc, 
			const ResourceArgs& args,
			std::string& rErr);
	private:
		std::vector<std::shared_ptr<IResourceFactory> >   m_factories;
	};

	class ResourceLoader : public BaseGameComponent
	{
	public:
		ResourceLoader();

		bool CreateType(const char* typeName);
		bool AddFactory(const std::shared_ptr<IResourceFactory>& pFactory);

		std::shared_ptr<IResource> LoadResource(const ResourceSource& rLoc, 
												const ResourceArgs& args);

		const char* GetResourceLoadError() const { return m_error.c_str(); }
	private:
		// Resource types
		std::unordered_map<std::string, ResourceTypeID>  m_resourceTypeMap;
		std::vector<ResourceType>    m_resTypes;
		
		// Error
		std::string m_error;
	};



}
