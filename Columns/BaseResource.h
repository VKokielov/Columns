#pragma once

#include "Resource.h"
#include <string>
#include <type_traits>

namespace geng
{

	template<typename Derived, typename Interface>
	class BaseResource : public Interface
	{
	public:
		static_assert(std::is_base_of_v<IResource, Interface>, "BaseResource: Interface must derive from IResource");

		// IDatum
		data::BaseDatumType GetDatumType() const override
		{
			return data::BaseDatumType::Object;
		}
		bool IsImmutable() const override
		{
			return false;
		}

		// Object type (from IObjectDatum)

		data::IObjectDatum* MakeView() const override
		{
			// By default do not support views -- the derived class should implement this function
			return nullptr;
		}

		data::IObjectDatum* Clone() const override
		{
			// If there's a copy constructor, use it, otherwise return null
			if constexpr (std::is_copy_constructible_v<Derived>)
			{
				return new Derived(*static_cast<const Derived*>(this));
			}
			else
			{
				return nullptr;
			}
		}
		
		bool GetRepresentation(std::string& rRepr) const override
		{
			rRepr = m_objDesc;
			return true;
		}
		// Resource type
		const char*  GetType() const override
		{
			return m_resType.c_str();
		}
		const char* GetObjectType() const override
		{
			return m_objType.c_str();
		}

		static std::string GetObjectType(const char* pType)
		{
			std::string retV{ "Resource_" };
			retV += pType;
			return retV;
		}

	protected:
		const std::string& GetTypeAsString() const { return m_resType; }
		const std::string& GetRawDescription() const { return m_rawDesc; }

		BaseResource(const char* pType, const char* pDesc)
			:m_resType(pType),
			m_rawDesc(pDesc)
		{ 
			// Construct the various other strings 
			m_objType = GetObjectType(pType);

			m_objDesc = "<resource-";
			m_objDesc += m_resType;
			m_objDesc += ':';
			m_objDesc += pDesc;
			m_objDesc += '>';
		}
	private:
		std::string m_resType;
		std::string m_objType;
		std::string m_objDesc;

		std::string m_rawDesc;
	};

	class BaseResourceFactory : public IResourceFactory
	{
	public:
		const char* GetType() const override;
		bool IsMyResource(const data::IDatum& resourceDesc) const override;
		std::shared_ptr<IResource> LoadResource(const data::IDatum& resourceDesc,
			IResourceLoader& loader,
			std::string& rErr) override;

	protected:
		const std::string& GetTypeAsString() const { return m_resType; }

		BaseResourceFactory(const char* pType)
			:m_resType(pType)
		{ }

	private:
		std::string m_resType;
	};

}