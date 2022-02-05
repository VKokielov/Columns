#pragma once

#include "UIImplDatumBase.h"

namespace geng::ui::impl
{
	class UIBaseClassDatumRefList : public UIDatumBase<IUIListDatum>
	{
	public:
		bool IsEmpty() const override;
		size_t GetLength() const override;
		bool GetEntries(const IUIDatum** ppChildArray,
			size_t idxOut,
			size_t startIdx,
			size_t endIdx) const override;

	protected:
		using UIDatumBase<IUIListDatum>::UIDatumBase;

		bool Add(const IUIDatum* pDatum, size_t insertBefore = data::LIST_NPOS);
		bool Remove(size_t idxRemove);
		bool Move(size_t idxSource, size_t idxTarget);

	private:
		std::vector<const IUIDatum*>   m_refList;
	};

	class UIDatumRefList : public UIBaseClassDatumRefList
	{
	public:
		using UIBaseClassDatumRefList::Add;
		using UIBaseClassDatumRefList::Remove;
		using UIBaseClassDatumRefList::Move;

		UIDatumRefList(UIElementBase* pOwner, ElementID elementId)
			:UIBaseClassDatumRefList(pOwner, elementId)
		{ }
	};
	
}