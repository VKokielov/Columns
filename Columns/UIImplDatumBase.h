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
			m_uiAddress.subElementId = pOwner->AddSubelement(this);
		}

		// UI-specific
		const UIAddress* GetAddress() const override
		{
			return &m_uiAddress;
		}

		// Datatree
		data::BaseDatumType GetUIDatumType() const override
		{
			if constexpr (std::is_base_of_v<IUIPrimitiveDatum,I>)
			{
				return data::BaseDatumType::Element;
			}
			else if constexpr (std::is_base_of_v<IUIDictionaryDatum,I>)
			{
				return data::BaseDatumType::Dictionary;
			}
			else if constexpr (std::is_base_of_v<IUIListDatum,I>)
			{
				return data::BaseDatumType::List;
			}
			else
			{
				static_assert(true, "Invalid base interface for UI datum");
			}
		}

		UIElementBase* GetOwner() const
		{
			return m_pOwner;
		}

		~UIDatumBase()
		{
			m_pOwner->EraseSubelement(m_uiAddress.subElementId);
		}

	private:

		UIAddress m_uiAddress;
		UIElementBase* m_pOwner{ nullptr };
	};


}