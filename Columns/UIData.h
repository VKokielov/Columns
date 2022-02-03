#pragma once

#include "UIBase.h"
#include "UIEngine.h"

namespace geng::ui
{

	class IUIDataManager : public IUIManager
	{
	public:
		virtual const IUIDatum* GetDataSubelement(const UIAddress& address) const = 0;
	};


}