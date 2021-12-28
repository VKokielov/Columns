#include "PathUtils.h"

std::filesystem::path
geng::filepath::FindInSearchPath(const std::vector<std::string>& paths,
	const std::vector<std::string>& names)
{
	std::filesystem::path retPath;

	for (const std::string& rPath : paths)
	{
		std::filesystem::path candidatePath{ rPath };
		
		bool foundPath{ false };
		for (const std::string& rName : names)
		{
			std::filesystem::path resultPath{ candidatePath };
			resultPath /= rName;
			if (std::filesystem::exists(resultPath))
			{
				retPath = resultPath;
				foundPath = true;
				break;
			}
		}
		if (foundPath)
		{
			break;
		}
	}

	return retPath;
}