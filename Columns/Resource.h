#pragma once

#include <string>
#include <cinttypes>

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

	class ResourceSource
	{
	public:
		ResourceSource(const char* pRestype, const char* pSourcePath)
			:m_sourceType(ResourceSourceType::Filename),
			m_typeName(pRestype),
			m_path(pSourcePath)
		{ }

		ResourceSource(const char* pRestype, const std::shared_ptr<IResource>& srcRes)
			:m_sourceType(ResourceSourceType::ResourceObj),
			m_typeName(pRestype),
			m_psrc(srcRes)
		{ }

		ResourceSourceType GetSourceType() const { return m_sourceType; }
		const char* GetResourceType() const { return m_typeName.c_str(); }
		const char* GetPath() const { return m_path.c_str(); }
		const std::shared_ptr<IResource>& GetSource() const { return m_psrc; }
	private:
		ResourceSourceType m_sourceType;
		std::string m_typeName;
		std::string m_path;
		std::shared_ptr<IResource>  m_psrc;
	};

	class IResource
	{
	public:
		virtual ~IResource() = default;
		virtual const char*  GetType() const = 0;
	};

	class IResourceFactory
	{
	public:
		virtual ~IResourceFactory() = default;
		virtual const char* GetType() const = 0;
		virtual bool IsMyResource(const ResourceSource& source) const = 0;
		virtual std::shared_ptr<IResource> LoadResource(const ResourceSource& source,
			const ResourceArgs& args, 
			std::string& rErr) = 0;
	};
}