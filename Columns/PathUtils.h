#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace geng::filepath
{

	// Finds an existing file within a set of search paths
	// Returns a default path if nothing was found
	std::filesystem::path
		FindInSearchPath(const std::vector<std::string>& paths,
			const std::vector<std::string>& names);


}