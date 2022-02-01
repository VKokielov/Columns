#pragma once

#include "IDataTree.h"
#include "UIElements.h"

namespace geng::ui
{


	// UI additional functions
	class IUIDataDecoration
	{
	public:
		virtual ~IUIDataDecoration() = default;
		virtual const UIAddress* GetAddress() = 0;
	};

	class IUIElementDatum : public data::IElementDatum, 
		                    public IUIDataDecoration
	{ };

	class IUIDictDatum : public data::IDictDatum,
		                 public IUIDataDecoration
	{ };

	class IUIListDatum : public data::IListDatum,
		public IUIDataDecoration
	{ };

	class IUIObjectDatum : public data::IObjectDatum,
		public IUIDataDecoration
	{ };
}