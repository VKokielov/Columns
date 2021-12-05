#pragma once

#include <memory>
#include <cstdio>

namespace geng
{
	struct FileCloser
	{
		void operator()(FILE* pFile)
		{
			fclose(pFile);
		}
	};

	using FileUPtr = std::unique_ptr<FILE, FileCloser>;

	// Open a file and at once put it inside a std::unique_ptr
	inline FileUPtr FUPtrOpen(const char* pFileName, const char* pMode)
	{
		FILE* pFile = fopen(pFileName, pMode);
		if (pFile)
		{
			return FileUPtr(pFile);
		}

		return FileUPtr();
	}

}