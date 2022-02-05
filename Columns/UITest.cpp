#include "UIImplDatumValue.h"
#include "UIImplDatumValueSet.h"

geng::ui::impl::UIDatumValue<std::string>  myString(nullptr, 1);

void TestF()
{
	geng::ui::impl::UIDatumValueSet<std::string> mySet{ nullptr, 1 };
}