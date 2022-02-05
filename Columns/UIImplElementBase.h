#pragma once

#include "UIBase.h"
#include "UIEngine.h"

namespace geng::ui::impl
{
	class UIElementBase : public IUIElement
	{
	public:
		SubelementID AddSubelement(const IUIDatum* pDatum);
		bool EraseSubelement(SubelementID id);
	};
}