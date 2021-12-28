#pragma once

#include "BaseGameComponent.h"
#include "Resource.h"
#include "DTConstruct.h"
#include "DTSimple.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <cinttypes>
#include <string>

namespace geng
{

	class ResourceLoader;

	class ResourceType
	{
	public:
		ResourceType(ResourceLoader& owner)
			:m_owner(owner)
		{ }
		void AddFactory(const std::shared_ptr<IResourceFactory>& pFactory);

		// Tries all available loaders until one succeeds
		std::shared_ptr<IResource> LoadResource(const data::IDatum& resDescriptor, 
			std::string& rErr);
	private:
		ResourceLoader& m_owner;
		std::vector<std::shared_ptr<IResourceFactory> >   m_factories;
	};

	class ResourceLoader : public BaseGameComponent,
		public IResourceLoader
	{
	public:
		ResourceLoader();

		bool AddFactory(const std::shared_ptr<IResourceFactory>& pFactory);

		std::shared_ptr<IResource> LoadResource(const data::IDatum& resDescriptor) override;
		const char* GetResourceLoadError() const override;
	private:
		// Resource types
		std::unordered_map<std::string, ResourceTypeID>  m_resourceTypeMap;
		std::vector<ResourceType>    m_resTypes;
		
		// Error
		std::string m_error;
	};

}
