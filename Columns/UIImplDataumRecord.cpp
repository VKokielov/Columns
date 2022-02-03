#include "UIImplDatumRecord.h"

namespace impl_ns = geng::ui::impl;

impl_ns::UIBaseClassDatumRecord::UIBaseClassDatumRecord(UIElementBase* pOwner,
	ElementID elementId,
	const UIBaseClassRecordRuntimeDef* pRuntimeDef)
	:UIDatumBase<IUIDictionaryDatum>(pOwner, elementId),
	m_pRuntimeDef(pRuntimeDef)
{

}

const geng::ui::IUIDatum* impl_ns::UIBaseClassDatumRecord::GetEntry(const char* pKey) const
{
	std::size_t idxChild;
	if (m_pRuntimeDef->GetEntry(pKey, idxChild))
	{
		return m_children[idxChild];
	}

	return nullptr;
}

bool impl_ns::UIBaseClassDatumRecord::Iterate(IUIDictCallback& rCallback) const
{
	// The definition gives the field names; the values are stored on the object
	size_t fieldIdx{ 0 };

	auto cbkFields = [&fieldIdx, &rCallback, this](const std::string& rFieldName)
	{
		if (!rCallback.OnEntry(rFieldName.c_str(), m_children[fieldIdx]))
		{
			return false;
		}
		++fieldIdx;
		return true;
	};

	// Go through the field names and use the count to match the field value
	m_pRuntimeDef->Iterate(cbkFields);
	return fieldIdx == m_children.size();
}

bool impl_ns::UIBaseClassDatumRecord::AddField(std::size_t fieldIdx, const IUIDatum* pDatum)
{
	if (fieldIdx >= m_children.size() || m_children[fieldIdx] != nullptr)
	{
		return false;
	}

	m_children[fieldIdx] = pDatum;
	return true;
}
