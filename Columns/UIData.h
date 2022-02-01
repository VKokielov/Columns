#pragma once

#include "UIElements.h"
#include "UIDatatree.h"

namespace geng::ui
{

	class IUIDataManager : public IUIManager
	{
	public:
		virtual const data::IDatum& GetDataRoot(ElementID elementId) = 0;
		virtual const data::IDatum* GetDataSubelement(const UIAddress& address) = 0;
	};


}