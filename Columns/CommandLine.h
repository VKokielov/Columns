#pragma once

#include <unordered_map>
#include <string>

namespace geng::cmdline
{

	struct ArgDesc
	{
		std::string argKey;
		std::string argShort;
	
		bool isOptional;
		int minArgCount;
		int maxArgCount;

		ArgDesc(const char* pKey,
				const char* pShort = "",
			    bool isOptional_ = true,
				int minArgCount_ = -1, 
			    int maxArgCount_ = -1)
			:argKey(pKey),
			 argShort(argShort),
			isOptional(isOptional_),
			minArgCount(minArgCount_),
			maxArgCount(maxArgCount_)
		{ }
	};

	struct ArgValues
	{
		std::vector<std::string>  vals;
	};

	bool ProcessArgs(const std::vector<ArgDesc>& argDescs,
		int argc,
		char** argv,
		std::unordered_map<std::string, ArgValues>& argMap,
		std::string& rErr);

	std::string GetUsageString(const std::vector<ArgDesc>& argDescs);

}