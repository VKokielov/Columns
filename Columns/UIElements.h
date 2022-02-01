#pragma once

#include <cinttypes>

namespace geng::ui
{

	using SubUserID = uint32_t;
	using SubscriptionID = uint64_t;
	using ElementID = uint64_t;
	using SubelementID = uint64_t;
	using EventID = uint32_t;

	using ElementClassID = uint64_t;
	using TabID = uint32_t;

	constexpr ElementID ROOT_ELEMENT_ID = 0;

	struct UIAddress
	{
		ElementID  elementId;
		SubelementID subElementId;
	};

	class IUIManager
	{
	public:
		virtual ~IUIManager() = default;
	};
}