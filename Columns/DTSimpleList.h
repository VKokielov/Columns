#pragma once
#include "IDataTree.h"

namespace geng::data::simple
{

	class List : public IListDatum
	{
	public:
		BaseDatumType GetDatumType() const override;
		size_t GetLength() const override;
		bool GetEntry(size_t idx, std::shared_ptr<IDatum>& rChild) const override;
		bool GetRange(std::vector<std::shared_ptr<IDatum> >& rSequence,
			size_t startIdx,
			size_t endIdx) const override;
		bool InsertEntry(const std::shared_ptr<IDatum>& pEntry,
			size_t indexBefore) override;
		bool SetEntry(const std::shared_ptr<IDatum>& pEntry,
			size_t indexAt) override;

	private:
		std::vector<std::shared_ptr<IDatum> > m_list;
	};

}