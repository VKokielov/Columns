#pragma once

#include "BaseResource.h"
#include <memory>

namespace geng
{
	class RawMemoryResource : public BaseResource<RawMemoryResource,IResource>
	{
	public:
		static const char* GetTypeName()
		{
			return "RawMemResource";
		}

		static const char* GetObjectTypeName()
		{
			return "Resource_RawMemResource";
		}
		
		RawMemoryResource(std::unique_ptr<uint8_t[]>&& pMem, size_t size, 
			const char* pDesc = nullptr);

		RawMemoryResource(const RawMemoryResource& other);

		size_t GetSize() const { return m_size; }
		const uint8_t* GetData() const { return m_pMem.get(); }
		uint8_t* GetData() { return m_pMem.get(); }

	private:
		std::unique_ptr<uint8_t[]>  m_pMem;
		size_t m_size;
	};

	class RawMemoryFactory : public BaseResourceFactory
	{
	public:
		RawMemoryFactory(size_t bufferSize);

		bool IsMyResource(const data::IDatum& resourceDesc) const override;
		std::shared_ptr<IResource> LoadResource(const data::IDatum& resourceDesc,
			IResourceLoader& loader,
			std::string& rErr) override;

	private:
		size_t m_bufferSize;
	};

}