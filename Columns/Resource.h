#pragma once

#include <string>
#include <cinttypes>

#include "IDataTree.h"

namespace geng
{

	using ResourceTypeID = uint64_t;

	struct ResourceArgs
	{

	};

	enum ResourceSourceType
	{
		Filename,
		ResourceObj
	};

	class IResource;

	class IResource : public data::IObjectDatum
	{
	public:
		virtual ~IResource() = default;
		virtual const char*  GetType() const = 0;
	};

	class IResourceLoader
	{
	public:
		virtual ~IResourceLoader() = default;
		virtual std::shared_ptr<IResource> LoadResource(const data::IDatum& resDescriptor) = 0;
		virtual const char* GetResourceLoadError() const = 0;
	};

	class IResourceFactory
	{
	public:
		virtual ~IResourceFactory() = default;
		virtual const char* GetType() const = 0;
		virtual bool IsMyResource(const data::IDatum& resDescriptor) const = 0;
		virtual std::shared_ptr<IResource> LoadResource(
			const data::IDatum& resDescriptor,
			IResourceLoader& loader,
			std::string& rErr) = 0;
	};
}