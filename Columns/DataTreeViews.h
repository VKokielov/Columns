#pragma once

#include "IDataTree.h"

namespace geng::data
{
	// View-wrappers for data

	template<typename I>
	class BaseView : public I
	{
	public:
		static_assert(std::is_base_of_v<IDatum, I>, "BaseView: I must derive from IDatum");

		BaseDatumType GetDatumType() const override
		{
			return m_datumType;
		}

		bool IsImmutable() const override
		{
			return true;
		}

	protected:
		BaseView(const std::shared_ptr<I>& pDatum,
			BaseDatumType datumType)
			:m_pDatum(pDatum),
			m_datumType(datumType)
		{ }

		template<typename F>
		std::invoke_result_t<F, const I*> 
			IfValid(F&& f) const
		{
			auto pDatum = m_pDatum.lock();

			if (pDatum)
			{
				return f(const_cast<const I*>(pDatum.get()));
			}
			
			throw DataStateException();
		}

	private:
		BaseDatumType m_datumType;
		std::weak_ptr<I> m_pDatum;
	};

	class ElementView : public BaseView<IElementDatum>
	{
	public:
		ElementView(const std::shared_ptr<IElementDatum>& pDatum)
			:BaseView<IElementDatum>(pDatum, BaseDatumType::Element)
		{ }

		ElementType GetElementType() const override;

		bool Get(bool& rDatum) const override;
		void Set(bool datum) override;

		bool Get(int8_t& rDatum) const override;
		void Set(int8_t datum) override;

		bool Get(uint8_t& rDatum) const override;
		void Set(uint8_t datum) override;

		bool Get(int16_t& rDatum) const override;
		void Set(int16_t datum) override;

		bool Get(uint16_t& rDatum) const override;
		void Set(uint16_t datum) override;

		bool Get(int32_t& rDatum) const override;
		void Set(int32_t datum) override;

		bool Get(uint32_t& rDatum) const override;
		void Set(uint32_t datum) override;

		bool Get(int64_t& rDatum) const override;
		void Set(int64_t datum) override;

		bool Get(uint64_t& rDatum) const override;
		void Set(uint64_t datum) override;

		bool Get(float& rDatum) const override;
		void Set(float datum) override;

		bool Get(double& rDatum) const override;
		void Set(double datum) override;

		bool Get(std::string& rDatum) const override;
		void Set(const char* datum) override;

		void Clear() override;
	private:
		template<typename T>
		bool GenericGet(T& rDatum) const;
	};

	class DictionaryView : public BaseView<IDictDatum>
	{
	public:
		DictionaryView(const std::shared_ptr<IDictDatum>& pDatum)
			:BaseView<IDictDatum>(pDatum, BaseDatumType::Dictionary)
		{ }

		bool IsEmpty() const;
		bool HasEntry(const char* pKey) const override;
		bool GetEntry(const char* pKey, std::shared_ptr<IDatum>& rChild) const override;
		bool Iterate(IDictCallback& rCallback) const override;

		bool SetEntry(const char* pKey, const std::shared_ptr<IDatum>& rChild) override;
	};

	class ListView : public BaseView<IListDatum>
	{
	public:
		ListView(const std::shared_ptr<IListDatum>& pDatum)
			:BaseView<IListDatum>(pDatum, BaseDatumType::List)
		{ }

		bool IsEmpty() const;
		size_t GetLength() const override;
		bool GetEntry(size_t idx, std::shared_ptr<IDatum>& rChild) const override;
		bool GetRange(std::vector<std::shared_ptr<IDatum> >& rSequence,
			size_t startIdx,
			size_t endIdx) const override;
		bool InsertEntry(const std::shared_ptr<IDatum>& pEntry,
			size_t indexBefore) override;
		bool SetEntry(const std::shared_ptr<IDatum>& pEntry,
			size_t indexAt) override;
	};

	inline std::shared_ptr<IElementDatum> MakeView(const std::shared_ptr<IElementDatum>& pDatum)
	{
		return std::make_shared<ElementView>(pDatum);
	}

	inline std::shared_ptr<IDictDatum> MakeView(const std::shared_ptr<IDictDatum>& pDatum)
	{
		return std::make_shared<DictionaryView>(pDatum);
	}

	inline std::shared_ptr<IListDatum> MakeView(const std::shared_ptr<IListDatum>& pDatum)
	{
		return std::make_shared<ListView>(pDatum);
	}

	inline std::shared_ptr<IDatum> MakeViewDynamic(const std::shared_ptr<IDatum>& pDatum)
	{
		if (pDatum->IsImmutable())
		{
			// Of course immutables are idempotent
			return pDatum;
		}

		BaseDatumType datumType = pDatum->GetDatumType();
		switch (datumType)
		{
		case BaseDatumType::Element:
			return MakeView(std::static_pointer_cast<IElementDatum>(pDatum));
		case BaseDatumType::Dictionary:
			return MakeView(std::static_pointer_cast<IDictDatum>(pDatum));
		case BaseDatumType::List:
			return MakeView(std::static_pointer_cast<IListDatum>(pDatum));
		case BaseDatumType::Object:
			std::shared_ptr<IDatum> pSafeDatum{ std::static_pointer_cast<IObjectDatum>(pDatum)->MakeView() };
			return pSafeDatum;
		}

		// The "null" case
		return std::shared_ptr<IDatum>();
	}
}