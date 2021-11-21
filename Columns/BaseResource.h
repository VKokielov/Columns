#pragma once

#include "Resource.h"
#include <string>

namespace geng
{

	class BaseResource : public IResource
	{
	public:
		const char*  GetType() const override;
	protected:
		const std::string& GetTypeAsString() const { return m_resType; }

		BaseResource(const char* pType)
			:m_resType(pType)
		{ }
	private:
		std::string m_resType;
	};

	class BaseResourceFactory : public IResourceFactory
	{
	public:
		const char* GetType() const override;
		bool IsMyResource(const ResourceSource& source) const override;
		std::shared_ptr<IResource> LoadResource(const ResourceSource& source,
			const ResourceArgs& args, std::string& rErr) override;

	protected:
		const std::string& GetTypeAsString() const { return m_resType; }

		BaseResourceFactory(const char* pType)
			:m_resType(pType)
		{ }

	private:
		std::string m_resType;
	};


}