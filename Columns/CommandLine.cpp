#include "CommandLine.h"
#include <unordered_set>
#include <sstream>

namespace
{
	// keyOutput is not overwritten unless this is an actual key
	bool GetKey(const char* pArg,
		std::string& keyOutput,
		bool& isShort)
	{
		// Instead of using C string manipulation (why?)
		std::string strArg{ pArg };

		if (strArg.size() >= 2
			&& strArg[0] == '-' 
			&& strArg[1] == '-')
		{
			isShort = false;
			keyOutput = strArg.substr(2, std::string::npos);
			return true;
		}
		else if (strArg.size() >= 1
			&& (strArg[0] == '-' || strArg[0] == '/'))
		{
			isShort = strArg[0] != '/';
			keyOutput = strArg.substr(1, std::string::npos);
			return true;
		}

		return false;
	}

}

bool geng::cmdline::ProcessArgs(const std::vector<ArgDesc>& argDescs,
	int argc,
	char** argv,
	std::unordered_map<std::string, ArgValues>& argMap,
	std::string& rErr)
{
	// Check the uniqueness of each key in the descriptions
	std::unordered_map<std::string,size_t> allKeys;
	std::unordered_set<std::string> requiredKeys;
	std::unordered_map<std::string,size_t> shortKeys;

	size_t argIdx{ 0 };

	for (const ArgDesc& argDesc : argDescs)
	{
		if (allKeys.count(argDesc.argKey) > 0)
		{
			rErr = "DEBUG:  Duplicate argument key: ";
			rErr += argDesc.argKey;
			return false;
		}

		if (!argDesc.argShort.empty() 
			&& shortKeys.count(argDesc.argShort) > 0)
		{
			rErr = "DEBUG:  Duplicate argument short key: ";
			rErr += argDesc.argShort;
			return false;
		}

		if (!argDesc.isOptional)
		{
			requiredKeys.emplace(argDesc.argKey);
		}

		allKeys[argDesc.argKey] = argIdx;
		shortKeys[argDesc.argShort] = argIdx;

		++argIdx;
	}
	
	// Go through the arguments
	std::string strCurKey;
	std::unordered_map<std::string, ArgValues>::iterator
		itCurKey = argMap.end();

	for (int argidx = 1; argidx < argc; ++argidx)
	{
		char* pArg = argv[argidx];
		bool isShortKey{ false };

		// Check the prefix of the string
		if (GetKey(pArg, strCurKey, isShortKey))
		{
			// If the key is short, find the corresponding long key
			// In any case check that the key is valid
			if (isShortKey)
			{
				auto itKeyRecord = shortKeys.find(strCurKey);
				if (itKeyRecord == shortKeys.end())
				{
					rErr += "Invalid short argument:";
					rErr += strCurKey;
					return false;
				}

				strCurKey = argDescs[itKeyRecord->second].argKey;
			}
			else
			{
				if (allKeys.count(strCurKey) == 0)
				{
					rErr += "Invalid argument:";
					rErr += strCurKey;
					return false;
				}
			}

			// This line works if the key is not required
			requiredKeys.erase(strCurKey);

			// This line works even if the key already exists
			auto insVal = argMap.emplace(strCurKey, ArgValues());
			itCurKey = insVal.first;
		}
		else
		{
			if (strCurKey.empty())
			{
				rErr = "Invalid first argument";
				return false;
			}

			itCurKey->second.vals.emplace_back(pArg);
		}
	}

	if (!requiredKeys.empty())
	{
		std::stringstream ssm;

		ssm << "The following arguments are required:\n";
		for (const std::string& strArg : requiredKeys)
		{
			ssm << "\t--" << strArg << '\n';
		}
		rErr = ssm.str();
		return false;
	}

	for (const ArgDesc& desc : argDescs)
	{
		auto itArgValues = argMap.find(desc.argKey);
		if (itArgValues != argMap.end())
		{
			// Test conformance
			if (desc.minArgCount != -1
				&& itArgValues->second.vals.size() < desc.minArgCount)
			{
				std::stringstream ssm;
				ssm << "Argument " << desc.argKey << " minimum argument count is " << desc.minArgCount;
				rErr = ssm.str();
				return false;
			}

			if (desc.maxArgCount != -1
				&& itArgValues->second.vals.size() > desc.maxArgCount)
			{
				std::stringstream ssm;
				ssm << "Argument " << desc.argKey << " maximum argument count is " << desc.maxArgCount;
				rErr = ssm.str();
				return false;
			}
		}
	}

	return true;
}

std::string geng::cmdline::GetUsageString(const std::vector<ArgDesc>& argDescs)
{
	std::stringstream ssm;
	ssm << "usage:\n";

	for (const ArgDesc& desc : argDescs)
	{
		std::stringstream ssmArgName;
		ssmArgName << "--" << desc.argKey;
		if (!desc.argShort.empty())
		{
			ssmArgName << " | -" << desc.argShort;
		}

		if (desc.isOptional)
		{
			ssm << "[" << ssmArgName.str() << "]";
		}
		else
		{
			ssm << ssmArgName.str();
		}

		if (desc.maxArgCount == -1 || desc.maxArgCount > 0)
		{
			if (desc.minArgCount != -1 && desc.minArgCount > 0)
			{
				ssm << " [...]";
			}
			else
			{
				ssm << " ...";
			}
		}
		ssm << ' ';
	}

	return ssm.str();
}