#pragma once
// A summary header including the three headers required for producing "simple"
// data trees

#include "DTSimpleList.h"
#include "DTSimpleDict.h"
#include "DTSimpleElement.h"
#include "DTConstruct.h"

namespace geng::data::simple
{
	struct Suite
	{
		// The names of the first two typedeffed types and the source types coincide,
		// but the target types are inside this struct whereas the source types
		// are in the namespace
		using Element = geng::data::simple::Element;
		using List = geng::data::simple::List;
		using Dict = geng::data::simple::Dictionary;
	};
}
