#pragma once

#include "Resource.h"
#include <string>
#include <type_traits>

namespace geng
{

	template<typename I>
	class BaseResource : public I
	{
	public:
		static_assert(std::is_base_of_v<IResource, I>, "BaseResource: I must derive from IResource");

		const char*  GetType() const override
		{
			return m_resType.c_str();
		}

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