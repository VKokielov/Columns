#include "UIImplDatumValue.h"
#include "UIImplDatumValueSet.h"
#include "LeveledHashmap.h"

geng::ui::impl::UIDatumValue<std::string>  myString(nullptr, 1);

void TestF()
{
	geng::ui::impl::UIDatumValueSet<std::string> mySet{ nullptr, 1 };
}

struct TestKey
{
	int intKey1;
	int intKey2;
	std::string strKey;

	template<size_t depth>
	struct Getter;

	template<>
	struct Getter<0>
	{
		static int Get(const TestKey& key)
		{
			return key.intKey1;
		}
	};

	template<>
	struct Getter<1>
	{
		static int Get(const TestKey& key)
		{
			return key.intKey2;
		}
	};

	template<>
	struct Getter<2>
	{
		static const std::string Get(const TestKey& key)
		{
			return key.strKey;
		}
	};
};

void TestLeveledHashmap()
{
	using namespace geng::lib;
	using namespace geng::lib::map_helpers;

	LeveledHashmap<std::string, 
				   int, int, std::string>  lmap;

	TestKey kTest;
	kTest.intKey1 = 10;
	kTest.intKey2 = 12;
	kTest.strKey = "bob";

	auto insVal = Insert(lmap, kTest);

	insVal.first->second = "employee";

	auto itCVal = Find(const_cast<const decltype(lmap)&>(lmap), kTest);
	auto itVal = Find(lmap, kTest);

	auto iterMap = [](int intKey1, int intkKey2, const std::string& strKey, const std::string& strValue)
	{
		return true;
	};

	IterateChildren(lmap, iterMap);
}