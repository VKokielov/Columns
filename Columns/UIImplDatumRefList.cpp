#include "UIImplDatumRefList.h"

namespace impl_ns = geng::ui::impl;


bool impl_ns::UIBaseClassDatumRefList::IsEmpty() const
{
	return m_refList.empty();
}

size_t impl_ns::UIBaseClassDatumRefList::GetLength() const
{
	return m_refList.size();
}

bool impl_ns::UIBaseClassDatumRefList::GetEntries(const IUIDatum** ppChildArray,
	size_t idxOut,
	size_t startIdx,
	size_t endIdx) const
{
	if (endIdx == data::LIST_NPOS)
	{
		endIdx = m_refList.size();
	}

	for (size_t idxSrc = startIdx; idxSrc < endIdx; ++idxSrc)
	{
		ppChildArray[idxOut] = m_refList[idxSrc];
		++idxOut;
	}

	return startIdx < endIdx;
}


bool impl_ns::UIBaseClassDatumRefList::Add(const geng::ui::IUIDatum* pDatum, size_t insertBefore)
{
	if (insertBefore > m_refList.size())
	{
		insertBefore = m_refList.size();
	}

	m_refList.insert(m_refList.begin() + insertBefore, pDatum);
	return true;
}

bool impl_ns::UIBaseClassDatumRefList::Remove(size_t idxRemove)
{
	if (idxRemove >= m_refList.size())
	{
		return false;
	}

	m_refList.erase(m_refList.begin() + idxRemove);
	return true;
}

bool impl_ns::UIBaseClassDatumRefList::Move(size_t idxSource, size_t idxTarget)
{
	if (idxSource >= m_refList.size() || idxTarget >= m_refList.size())
	{
		return false;
	}

	if (idxSource != idxTarget)
	{
		const IUIDatum* pObj = m_refList[idxSource];
		m_refList.erase(m_refList.begin() + idxSource);

		if (idxTarget > idxSource)
		{
			--idxTarget;
		}

		m_refList.insert(m_refList.begin() + idxTarget, pObj);
	}

	return true;
}