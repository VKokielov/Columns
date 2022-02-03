#pragma once

#include "UIBase.h"
#include "IDataTree.h"
#include <memory>

namespace geng::ui
{
	enum class EngineResult
	{
		OK
	};

	class IUIDatum
	{
	public:
		virtual ~IUIDatum() = default;

		virtual const UIAddress* GetAddress() const = 0;
		virtual data::BaseDatumType GetUIDatumType() const = 0;
	};

	class IUIPrimitiveDatum : public IUIDatum
	{
	public:
		virtual data::ElementType GetPrimitiveType() const = 0;
	};

	template<typename T>
	class IUITypedPrimitiveDatum : public IUIPrimitiveDatum
	{
	public:
		virtual T GetData() const = 0;
	};

	class IUIStringDatum : public IUIPrimitiveDatum
	{
	public:
		virtual const char* GetData() const = 0;
	};

	class IUIDictCallback
	{
	public:
		virtual ~IUIDictCallback() = default;
		virtual bool OnEntry(const char* pKey, const IUIDatum* pChild) = 0;
	};

	class IUIDictionaryDatum : public IUIDatum
	{
	public:
		virtual const IUIDatum* GetEntry(const char* pKey) const = 0;
		virtual bool Iterate(IUIDictCallback& rCallback) const = 0;
	};

	class IUIListDatum : public IUIDatum
	{
		virtual bool IsEmpty() const = 0;
		virtual size_t GetLength() const = 0;
		virtual bool GetEntries(size_t idx, const IUIDatum** ppChildArray,
			size_t startIdx = 0, size_t count = 0) const = 0;
	};

	class IUIElement
	{
	public:
		virtual ~IUIElement() = default;

		// Get data
		virtual const IUIDatum* GetDataSubelement(SubelementID subElementId) const = 0;
	};

	class IUIElementFactory
	{
	public:
		virtual const UIClassDef& GetClassDef() const = 0;
		virtual IUIElement* CreateElement(const data::IDatum* elementArgs) = 0;
	};

	struct AddClassRequest
	{
		// [in]
		std::shared_ptr<IUIElementFactory> pFactory;
		// [out]
		ElementClassID out_classId;
		EngineResult out_result;
	};

	class IUIEngine
	{
	public:
		virtual bool AddClass(AddClassRequest* pRequests,
			size_t nCount) = 0;
	};

}