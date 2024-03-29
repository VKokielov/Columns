#include "DataTreeViews.h"
#include <algorithm>

// ElementView
geng::data::ElementType geng::data::ElementView::GetElementType() const
{
	return IfValid([](const IElementDatum* pBase){
		return pBase->GetElementType();
	});

}

template<typename T>
bool geng::data::ElementView::GenericGet(T& rDatum) const
{
	return IfValid([&rDatum](const IElementDatum* pBase) {
		return pBase->Get(rDatum);
	});
}

bool geng::data::ElementView::Get(bool& rDatum) const 
{
	return GenericGet(rDatum);
}

void geng::data::ElementView::Set(bool datum) 
{
	throw DataStateException();
}

bool geng::data::ElementView::Get(int8_t& rDatum) const 
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(int8_t datum) 
{
	throw DataStateException();
}

bool geng::data::ElementView::Get(uint8_t& rDatum) const 
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(uint8_t datum) 
{
	throw DataStateException();
}

bool geng::data::ElementView::Get(int16_t& rDatum) const 
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(int16_t datum)
{ 
	throw DataStateException();
}

bool geng::data::ElementView::Get(uint16_t& rDatum) const 
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(uint16_t datum) 
{ 
	throw DataStateException();
}

bool geng::data::ElementView::Get(int32_t& rDatum) const 
{ 
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(int32_t datum) 
{ 
	throw DataStateException();
}

bool geng::data::ElementView::Get(uint32_t& rDatum) const
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(uint32_t datum) 
{
	throw DataStateException();
}

bool geng::data::ElementView::Get(int64_t& rDatum) const
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(int64_t datum)
{
	throw DataStateException();
}
bool geng::data::ElementView::Get(uint64_t& rDatum) const 
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(uint64_t datum) 
{
	throw DataStateException();
}

bool geng::data::ElementView::Get(float& rDatum) const 
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(float datum) 
{
	throw DataStateException();
}
bool geng::data::ElementView::Get(double& rDatum) const 
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(double datum) 
{
	throw DataStateException();
}
bool geng::data::ElementView::Get(std::string& rDatum) const 
{
	return GenericGet(rDatum);
}
void geng::data::ElementView::Set(const char* datum) 
{ 
	throw DataStateException();
}
void geng::data::ElementView::Clear()
{
	throw DataStateException();
}

// DictionaryView
bool geng::data::DictionaryView::IsEmpty() const
{
	return IfValid([](const IDictDatum* pDatum)
	{
		return pDatum->IsEmpty();
	}
	);
}

bool geng::data::DictionaryView::HasEntry(const char* pKey) const
{
	return IfValid([pKey](const IDictDatum* pDatum)
	{
		return pDatum->HasEntry(pKey);
	}
	);
}
bool geng::data::DictionaryView::GetEntry(const char* pKey, std::shared_ptr<IDatum>& rChild) const
{
	return IfValid([pKey, &rChild](const IDictDatum* pDatum)
	{
		std::shared_ptr<IDatum> pUnsafe;
		if (pDatum->GetEntry(pKey, pUnsafe))
		{
			rChild = MakeViewDynamic(pUnsafe);
			return true;
		}
		return false;
	});
}
bool geng::data::DictionaryView::Iterate(IDictCallback& rCallback) const
{
	return IfValid([&rCallback](const IDictDatum* pDatum)
	{
		struct SafeServer : public IDictCallback
		{
			SafeServer(IDictCallback& rCallback_)
				:m_rCallback(rCallback_)
			{ }

			bool OnEntry(const char* pKey, const std::shared_ptr<IDatum>& pChild) override
			{
				return m_rCallback.OnEntry(pKey, MakeViewDynamic(pChild));
			}

			IDictCallback& m_rCallback;
		};

		SafeServer safeServer(rCallback);
		return pDatum->Iterate(safeServer);
	}
	);
}

bool geng::data::DictionaryView::SetEntry(const char* pKey, const std::shared_ptr<IDatum>& rChild)
{
	throw DataStateException();
}

bool geng::data::ListView::IsEmpty() const
{
	return IfValid([](const IListDatum* pDatum)
	{
		return pDatum->IsEmpty();
	}
	);
}
size_t geng::data::ListView::GetLength() const
{
	return IfValid([](const IListDatum* pDatum) {
		return pDatum->GetLength();
	}
	);
}
bool geng::data::ListView::GetEntry(size_t idx, std::shared_ptr<IDatum>& rChild) const
{
	return IfValid([idx,&rChild](const IListDatum* pDatum) {
		std::shared_ptr<IDatum> pUnsafe;
		if (pDatum->GetEntry(idx, pUnsafe))
		{
			rChild = MakeViewDynamic(pUnsafe);
			return true;
		}
		return false;
	}
	);
}
bool geng::data::ListView::GetRange(std::vector<std::shared_ptr<IDatum> >& rSequence,
	size_t startIdx,
	size_t endIdx) const
{
	return IfValid([&rSequence,startIdx,endIdx](const IListDatum* pDatum) {
		std::vector<std::shared_ptr<IDatum> > unsafeData;
		if (!pDatum->GetRange(unsafeData, startIdx, endIdx))
		{
			return false;
		}
		for (const std::shared_ptr<IDatum>& pUnsafe : unsafeData)
		{
			rSequence.emplace_back(MakeViewDynamic(pUnsafe));
		}
		return true;
	}
	);
}
bool geng::data::ListView::InsertEntry(const std::shared_ptr<IDatum>& pEntry,
	size_t indexBefore)
{
	throw DataStateException();
}
bool geng::data::ListView::SetEntry(const std::shared_ptr<IDatum>& pEntry,
	size_t indexAt)
{
	throw DataStateException();
}