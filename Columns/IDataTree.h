#pragma once

#include <string>
#include <cinttypes>
#include <memory>
#include <limits>
#include <vector>

namespace geng::data
{

	enum class BaseDatumType
	{
		Element,
		Dictionary,
		List
	};

	enum class ElementType
	{
		None,
		Boolean,
		Int8,
		UInt8,
		Int16,
		UInt16,
		Int32,
		UInt32,
		Int64,
		UInt64,
		Float,
		Double,
		String
	};

	class DataStateException { };

	class IDatum
	{
	public:
		virtual ~IDatum() = default;
		virtual BaseDatumType GetDatumType() const = 0;
	};

	class IElementDatum : public IDatum
	{
	public:
		virtual ElementType GetElementType() const = 0;

		virtual bool Get(bool& rDatum) const = 0;
		virtual void Set(bool datum) = 0;

		virtual bool Get(int8_t& rDatum) const = 0;
		virtual void Set(int8_t datum) = 0;

		virtual bool Get(uint8_t& rDatum) const = 0;
		virtual void Set(uint8_t datum) = 0;

		virtual bool Get(int16_t& rDatum) const = 0;
		virtual void Set(int16_t datum) = 0;

		virtual bool Get(uint16_t& rDatum) const = 0;
		virtual void Set(uint16_t datum) = 0;

		virtual bool Get(int32_t& rDatum) const = 0;
		virtual void Set(int32_t datum) = 0;

		virtual bool Get(uint32_t& rDatum) const = 0;
		virtual void Set(uint32_t datum) = 0;

		virtual bool Get(uint64_t& rDatum) const = 0;
		virtual void Set(uint64_t datum) = 0;

		virtual bool Get(float& rDatum) const = 0;
		virtual void Set(float datum) = 0;

		virtual bool Get(double& rDatum) const = 0;
		virtual void Set(double datum) = 0;

		virtual bool Get(std::string& rDatum) const = 0;
		virtual void Set(const char* datum) = 0;

		virtual void Clear() = 0;  // reset to none
	};

	class IDictCallback
	{
	public:
		virtual ~IDictCallback() = 0;
		virtual bool OnEntry(const char* pKey, const std::shared_ptr<IDatum>& pChild) = 0;
	};

	class IDictDatum : public IDatum
	{
	public:
		virtual bool HasEntry(const char* pKey) const = 0;
		virtual bool GetEntry(const char* pKey, std::shared_ptr<IDatum>& rChild) const = 0;
		virtual bool Iterate(IDictCallback& rCallback) const = 0;

		virtual bool SetEntry(const char* pKey, const std::shared_ptr<IDatum>& rChild) = 0;
	};

	constexpr size_t LIST_NPOS = std::numeric_limits<size_t>::max();

	class IListDatum : public IDatum
	{
	public:
		virtual size_t GetLength() const = 0;
		virtual bool GetEntry(size_t idx, std::shared_ptr<IDatum>& rChild) const = 0;
		// Iteration would not be faster than calling GetEntry() a number of times, but 
		// GetRange may be, although the interface must be hybridized
		virtual bool GetRange(std::vector<std::shared_ptr<IDatum> >& rSequence,
							  size_t startIdx = 0, 
							  size_t endIdx = LIST_NPOS) const = 0;

		virtual bool InsertEntry(const std::shared_ptr<IDatum>& pEntry,
			size_t indexBefore = LIST_NPOS) = 0;
		virtual bool SetEntry(const std::shared_ptr<IDatum>& pEntry,
			size_t indexAt) = 0;
	};

	class IDatumFactories
	{
	public:
		virtual IDatum* CreateElement() = 0;
		virtual IDatum* CreateDictionary() = 0;
		virtual IDatum* CreateList() = 0;
	};

}