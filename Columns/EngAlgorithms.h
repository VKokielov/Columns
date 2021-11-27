#pragma once

#include <algorithm>

namespace geng
{

	// Universal add_unique function -- first check that something is not in the container
	// then add
	template<typename IterIn, typename IterOut, typename T>
	void add_unique(IterIn begin, IterIn end, IterOut insert, T&& toAdd)
	{
		if (std::find(begin, end, toAdd) == end)
		{
			*insert = toAdd;
		}
	}
}