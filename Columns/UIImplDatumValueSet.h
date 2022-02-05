#pragma once

#include "UIImplDatumBase.h"
#include "UIImplDatumValue.h"
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace geng::ui::impl
{
	// For normal values
	template<typename T>
	struct InputType
	{
		using type = T;
	};

	// For strings
	template<>
	struct InputType<std::string>
	{
		using type = const std::string&;
	};

	template<typename T>
	using InputType_t = typename InputType<T>::type;

	template<typename T>
	class UIBaseClassDatumValueSet : public UIDatumBase<IUIListDatum>
	{
	private:
		using TBase = UIDatumBase<IUIListDatum>;
	public:
		bool IsEmpty() const override { return m_dataList.empty(); }
		size_t GetLength() const override { return m_dataList.size(); }

		bool GetEntries(const IUIDatum** ppChildArray,
			size_t idxOut,
			size_t startIdx, size_t endIdx) const
		{
			if (endIdx == data::LIST_NPOS)
			{
				endIdx = m_dataList.size();
			}

			for (size_t i = startIdx; i < endIdx; ++i)
			{
				ppChildArray[idxOut] = &m_dataList[i];
				++idxOut;
			}

			return startIdx < endIdx;
		}

	protected:
		using UIDatumBase<IUIListDatum>::UIDatumBase;

		bool Add(InputType_t<T> value)
		{
			auto insVal = m_uniquenessMap.emplace(value, m_dataList.size());
			if (insVal.second)
			{
				// Actually add
				m_dataList.emplace_back(TBase::GetOwner(), TBase::GetAddress()->elementId);
				return true;
			}

			return false;
		}

		bool Remove(InputType_t<T> value)
		{
			// Removing is tricker.
			auto itValue = m_uniquenessMap.find(value);

			if (itValue != m_uniquenessMap.end())
			{
				size_t idx = itValue->second;

				size_t idxLast = m_dataList.size() - 1;

				if (idx < idxLast)
				{
					// Get the value at -1 and "swap" it in the map
					InputType_t<T> lastValue = m_dataList[idxLast];
					auto itLastValue = m_uniquenessMap[lastValue.GetValue()];
					itLastValue->second = idx;
					std::iter_swap(m_dataList.begin() + idx, m_dataList.begin() + idxLast);
				}

				// note: there is overhead here -- we accept it
				m_dataList.pop_back();

				return true;
			}

			return false;
		}

		bool IsInSet(InputType_t<T> value)
		{
			return m_uniquenessMap.count(value) > 0;
		}

	private:
		std::vector<UIDatumValue<T> >  m_dataList;
		std::unordered_map<T, size_t>  m_uniquenessMap;
	};

	template<typename T>
	class UIDatumValueSet : public UIBaseClassDatumValueSet<T>
	{
	public:
		UIDatumValueSet(UIElementBase* pOwner, ElementID elementId)
			:UIBaseClassDatumValueSet<T>(pOwner, elementId)
		{ }

		using UIBaseClassDatumValueSet<T>::Add;
		using UIBaseClassDatumValueSet<T>::Remove;
		using UIBaseClassDatumValueSet<T>::IsInSet;

	};

}