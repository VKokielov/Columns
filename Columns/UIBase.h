#pragma once

#include <cinttypes>
#include <string>

namespace geng::ui
{

	using SubUserID = uint32_t;
	using SubscriptionID = uint64_t;
	using ElementID = uint32_t;
	using SubelementID = uint32_t;
	using StringID = uint32_t;

	using ElementClassID = uint64_t;
	using AgentID = uint32_t;

	constexpr StringID ERR_STRING = ~(0x0L);
	constexpr ElementID ROOT_ELEMENT_ID = 0;
	constexpr SubelementID SUBELEMENT_NONE = ~(0x0LL);

	struct UIAddress
	{
		ElementID  elementId;
		SubelementID subElementId;
	};

	struct UIClassDef
	{
		std::string className;
	};

	class IUIManager
	{
	public:
		virtual ~IUIManager() = default;
	};
}