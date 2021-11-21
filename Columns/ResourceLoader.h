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

	// Utility function for safer resource loading, with automatic casting and correct arguments
	template<typename R>
	struct ResourceTraits
	{
		using TArgs = ResourceArgs;
	};

	template<typename ResClass, typename SourceArg>
	std::shared_ptr<ResClass> LoadResource(ResourceLoader* pLoader,
		SourceArg&& resSrcArg,
		const typename ResourceTraits<ResClass>::TArgs& resArgs)
	{
		ResourceSource resSrc(ResClass::GetTypeName(), std::forward<SourceArg>(resSrcArg));

		auto pResource = pLoader->LoadResource(resSrc, resArgs);
		if (pResource)
		{
			return std::static_pointer_cast<ResClass>(pResource);
		}

		static std::shared_ptr<ResClass> emptyPtr;
		return emptyPtr;
	}

	template<typename ResClass, typename SourceArg>
	std::shared_ptr<ResClass> LoadResource(ResourceLoader* pLoader,
		SourceArg&& resSrcArg)
	{
		static_assert(std::is_same_v<typename ResourceTraits<ResClass>::TArgs, ResourceArgs>,
			"The no-resource-arguments overload of LoadResource can only be used for resources that take the base-class ResourceArgs as arguments");

		ResourceArgs args;
		return LoadResource<ResClass>(pLoader, std::forward<SourceArg>(resSrcArg), args);
	}

}
