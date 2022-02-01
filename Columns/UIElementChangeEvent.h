#pragma once

#include "UIEvent.h"
#include "UIDatatree.h"

namespace geng::ui
{
	class UIElementChangeEvent : public UIEvent
	{
	public:
		data::ElementType GetElementType() const;
	};

	template<typename T>
	class UITypedElementChangeEvent : public UIElementChangeEvent
	{
	public:
		const T& GetPreviousValue() const;
		const T& GetNewValue() const;
	};


}