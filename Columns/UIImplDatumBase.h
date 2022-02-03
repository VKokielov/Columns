#pragma once

#include "UIImplElementBase.h"

#include <unordered_map>
#include <type_traits>

namespace geng::ui::impl
{

	template<typename I>
	class UIDatumBase : public I
	{
	protected:
		UIDatumBase(UIElementBase* pOwner, ElementID elementId)
			:m_pOwner(pOwner)
		{
			m_uiAddress.elementId = elementId;
			m_uiAddress.subElementId = SUBELEMENT_NONE;
		}

		// UI-specific
		const UIAddress* GetAddress() const override
		{
			return &m_uiAddress;
		}

		// Datatree
		data::BaseDatumType GetDatumType() const override
		{
			if constexpr (std::is_base_of_v<IUIPrimitiveDatum>)
			{
				return data::BaseDatumType::Element;
			}
			else if constexpr (std::is_base_of_v<IUIDictionaryDatum>)
			{
				return data::BaseDatumType::Dictionary;
			}
			else if constexpr (std::is_base_of_v<IUIListDatum>)
			{
				return data::BaseDatumType::List;
			}
			else
			{
				static_assert(true, "Invalid base interface for UI datum");
			}
		}

		~UIDatumBase()
		{
			for (auto& rDatumPair : m_elements)
			{
				m_pOwner->EraseSubelement(rDatumPair.second);
				// NOTE:  In this case, the subElementId is NOT set to SUBELEMENT_NONE!
				// This could result in inconsistencies or crashes if someone besides
				// the parent subelement is holding a reference to the subelement in question
				
				// this may be remediable in the derived classes
			}
		}

		template<typename U>
		bool AddSubelement(UIDatumBase<U>* pDatum)
		{
			auto insResult = m_elements.emplace(pDatum, 0);
			if (!insResult.second)
			{
				// Already there
				return true;
			}

			SubelementID eid = m_pOwner->AddSubelement(pDatum);
			
			insResult.first->second = eid;
			pDatum->m_uiAddress.subElementId = eid;
			return true;
		}

		template<typename U>
		bool RemoveSubelement(UIDatumBase<U>* pDatum)
		{
			auto itDatum = m_elements.find(pDatum);
			if (itDatum == m_elements.end())
			{
				return true;
			}

			m_pOwner->EraseSubelement(itDatum.second);
			m_elements.erase(itDatum);

			pDatum->m_uiAddress.subElementId = SUBELEMENT_NONE;
			return true;
		}

	private:

		template<typename U>
		friend class UIDatumBase;

		UIAddress m_uiAddress;
		UIElementBase* m_pOwner{ nullptr };
		std::unordered_map<IUIDatum*, SubelementID>  m_elements;
	};


}